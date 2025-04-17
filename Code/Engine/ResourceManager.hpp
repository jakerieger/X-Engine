#pragma once

#include <typeindex>

#include "Common/Types.hpp"
#include "ArenaAllocator.hpp"
#include "EntityId.hpp"
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
        virtual ~ResourceLoaderBase()                                                               = default;
        virtual ResourceBase* Load(RenderContext& context, ArenaAllocator& allocator, const u64 id) = 0;
    };

    template<typename T>
    class ResourceLoader : public ResourceLoaderBase {
    public:
        ResourceBase* Load(RenderContext& context, ArenaAllocator& allocator, const u64 id) override {
            void* memory = allocator.Allocate(sizeof(Resource<T>), alignof(Resource<T>));
            if (!memory) return nullptr;
            return new (memory) Resource<T>(LoadImpl(context, id));
        }

    private:
        virtual T LoadImpl(RenderContext& context, const u64 id) = 0;
    };

    template<typename T>
    class ResourceHandle;

    class ResourceManager {
        X_CLASS_PREVENT_MOVES_COPIES(ResourceManager)

        ArenaAllocator mAllocator;
        RenderContext& mRenderContext;
        std::unordered_map<u64, ResourceBase*> mResources;
        std::unordered_map<std::type_index, unique_ptr<ResourceLoaderBase>> mLoaders;

    public:
        explicit ResourceManager(RenderContext& context, const size_t arenaSize = X_GIGABYTES(1))
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
        bool LoadResource(const u64 id) {
            if (mResources.contains(id)) {
                return true;  // asset already exists
            }

            auto loaderIt = mLoaders.find(std::type_index(typeid(T)));
            if (loaderIt == mLoaders.end()) {
                return false;  // no registered loader for type
            }

            ResourceBase* resource = loaderIt->second->Load(mRenderContext, mAllocator, id);
            if (!resource) {
                return false;  // loading failed
            }

            mResources[id] = resource;
            return true;
        }

        template<typename T>
        ResourceHandle<T> FetchResource(const u64 id) {
            auto it = mResources.find(id);
            if (it == mResources.end()) { return ResourceHandle<T> {}; }

            Resource<T>* typedResource = DCAST<Resource<T>*>(it->second);
            if (!typedResource) { return {}; }

            return ResourceHandle<T>(this, id, &typedResource->data);
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
        u64 mId {0};
        T* mData;

    public:
        ResourceHandle() : mManager(nullptr), mData(nullptr) {}

        ResourceHandle(ResourceManager* manager, const u64 id, T* data) : mManager(manager), mId(id), mData(data) {}

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
            return (mManager != nullptr) && (mData != nullptr) && (mId != 0) && (EntityId {mId}.Valid());
        }
    };
}  // namespace x