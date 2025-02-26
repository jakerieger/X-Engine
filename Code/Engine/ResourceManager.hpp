#pragma once

#include <typeindex>

#include "Common/Types.hpp"
#include "Memory.hpp"
#include "ArenaAllocator.hpp"
#include "RenderContext.hpp"

namespace x {
    class ResourceLoaderBase;

    class ResourceRegistry {
        friend class ResourceManager;

        using LoaderFactory = unique_ptr<ResourceLoaderBase> (*)();
        using TypeMap       = std::unordered_map<std::type_index, LoaderFactory>;

        static TypeMap& GetLoaderFactories() {
            static TypeMap factories;
            return factories;
        }

    public:
        template<typename ResourceT, typename LoaderT>
        struct Registrar {
            Registrar() {
                ResourceRegistry::GetLoaderFactories()[std::type_index(typeid(ResourceT))] =
                  []() -> unique_ptr<ResourceLoaderBase> { return make_unique<LoaderT>(); };
            }
        };
    };

// Necessary for __LINE__ to expand when used in REGISTER_RESOURCE_LOADER
#define CONCAT_IMPL(x, y) x##y
#define CONCAT(x, y) CONCAT_IMPL(x, y)

#define X_REGISTER_RESOURCE_LOADER(ResourceType, LoaderType)                                                           \
    namespace {                                                                                                        \
        static const x::ResourceRegistry::Registrar<ResourceType, LoaderType> CONCAT(resourceRegistrar, __LINE__);     \
    }

    class ResourceBase {
    public:
        virtual ~ResourceBase() = default;
    };

    template<typename T>
    class Resource : public ResourceBase {
    public:
        T data;

        template<typename... Args>
        Resource(Args&&... args) : data(std::forward<Args>(args)...) {}
    };

    class ResourceLoaderBase {
    public:
        virtual ~ResourceLoaderBase()                                                                  = default;
        virtual ResourceBase* Load(RenderContext& context, ArenaAllocator& allocator, const str& path) = 0;
    };

    template<typename T>
    class ResourceLoader : public ResourceLoaderBase {
    public:
        ResourceBase* Load(RenderContext& context, ArenaAllocator& allocator, const str& path) override {
            void* memory = allocator.Allocate(sizeof(Resource<T>), alignof(Resource<T>));
            if (!memory) return None;
            return new (memory) Resource<T>(LoadImpl(context, path));
        }

    private:
        virtual T LoadImpl(RenderContext& context, const str& path) = 0;
    };

    template<typename T>
    class ResourceHandle;

    class ResourceManager {
        X_CLASS_PREVENT_MOVES_COPIES(ResourceManager)

        ArenaAllocator mAllocator;
        RenderContext& mRenderContext;
        std::unordered_map<str, ResourceBase*> mResources;
        std::unordered_map<std::type_index, unique_ptr<ResourceLoaderBase>> mLoaders;

    public:
        explicit ResourceManager(RenderContext& context, const size_t arenaSize = Memory::BYTES_1GB)
            : mAllocator(arenaSize), mRenderContext(context) {
            for (const auto& [type, factory] : ResourceRegistry::GetLoaderFactories()) {
                mLoaders[type] = factory();
            }
        }

        ~ResourceManager() {
            mAllocator.Reset();
        }

        template<typename T, typename LoaderT>
        void RegisterLoader() {
            mLoaders[std::type_index(typeid(T))] = make_unique<LoaderT>();
        }

        template<typename T>
        bool LoadResource(const str& path) {
            if (mResources.contains(path)) {
                return true;  // loader already exists
            }

            auto loaderIt = mLoaders.find(std::type_index(typeid(T)));
            if (loaderIt == mLoaders.end()) {
                return false;  // no registered loader for type
            }

            ResourceBase* resource = loaderIt->second->Load(mRenderContext, mAllocator, path);
            if (!resource) {
                return false;  // loading failed
            }

            mResources[path] = resource;
            return true;
        }

        template<typename T>
        std::optional<ResourceHandle<T>> FetchResource(const str& path) {
            auto it = mResources.find(path);
            if (it == mResources.end()) {
                return {};  // nullopt
            }

            Resource<T>* typedResource = DCAST<Resource<T>*>(it->second);
            if (!typedResource) { return {}; }

            return ResourceHandle<T>(this, path, &typedResource->data);
        }

        void Clear() {
            mResources.clear();
            mAllocator.Reset();
        }

        const ArenaAllocator& GetAllocator() {
            return mAllocator;
        }
    };

    template<typename T>
    class ResourceHandle {
        ResourceManager* mManager;
        str mPath;
        T* mData;

    public:
        ResourceHandle() : mManager(None), mData(None) {}

        ResourceHandle(ResourceManager* manager, const str& path, T* data)
            : mManager(manager), mPath(path), mData(data) {}

        T* Get() {
            return mData;
        }

        const T* Get() const {
            return mData;
        }

        T* operator->() {
            return mData;
        }

        const T* operator->() const {
            return mData;
        }

        [[nodiscard]] bool Valid() const {
            return (mManager != None) && (mData != None) && (!mPath.empty());
        }
    };
}  // namespace x