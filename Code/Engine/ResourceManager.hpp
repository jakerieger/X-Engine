#pragma once

#include <typeindex>

#include "Common/Types.hpp"
#include "Memory.hpp"
#include "ArenaAllocator.hpp"

namespace x {
    class ResourceLoaderBase;

    class ResourceRegistry {
        friend class ResourceManager;

        using LoaderFactory = unique_ptr<ResourceLoaderBase>(*)();
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
                    []() -> unique_ptr<ResourceLoaderBase> {
                        return make_unique<LoaderT>();
                    };
            }
        };
    };

    #define CONCAT_IMPL(x, y) x##y
    #define CONCAT(x, y) CONCAT_IMPL(x, y)

    #define REGISTER_RESOURCE_LOADER(ResourceType, LoaderType) \
    namespace { \
        static const x::ResourceRegistry::Registrar<ResourceType, LoaderType> CONCAT(resourceRegistrar, __LINE__); \
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
        virtual ~ResourceLoaderBase() = default;
        virtual ResourceBase* Load(ArenaAllocator& allocator, const str& path) = 0;
    };

    template<typename T>
    class ResourceLoader : public ResourceLoaderBase {
    public:
        ResourceBase* Load(ArenaAllocator& allocator, const str& path) override {
            void* memory = allocator.Allocate(sizeof(Resource<T>), alignof(Resource<T>));
            if (!memory)
                return None;
            return new(memory) Resource<T>(LoadImpl(path));
        }

    private:
        virtual T LoadImpl(const str& path) = 0;
    };

    class ResourceManager {
        CLASS_PREVENT_MOVES_COPIES(ResourceManager)

        ArenaAllocator _allocator;
        std::unordered_map<str, ResourceBase*> _resources;
        std::unordered_map<std::type_index, unique_ptr<ResourceLoaderBase>> _loaders;

    public:
        ResourceManager(const size_t arenaSize = Memory::BYTES_256MB) : _allocator(arenaSize) {
            for (const auto& [type, factory] : ResourceRegistry::GetLoaderFactories()) {
                _loaders[type] = factory();
            }
        }

        ~ResourceManager() {
            _allocator.Reset();
        }

        template<typename T, typename LoaderT>
        void RegisterLoader() {
            _loaders[std::type_index(typeid(T))] = make_unique<LoaderT>();
        }

        template<typename T>
        bool LoadResource(const str& path) {
            if (_resources.contains(path)) {
                return true; // loader already exists
            }

            auto loaderIt = _loaders.find(std::type_index(typeid(T)));
            if (loaderIt == _loaders.end()) {
                return false; // no registered loader for type
            }

            ResourceBase* resource = loaderIt->second->Load(_allocator, path);
            if (!resource) {
                return false; // loading failed
            }

            _resources[path] = resource;
            return true;
        }

        template<typename T>
        std::optional<T*> FetchResource(const str& path) {
            auto it = _resources.find(path);
            if (it == _resources.end()) {
                return {}; // nullopt
            }

            Resource<T>* typedResource = DCAST<Resource<T>*>(it->second);
            if (!typedResource) {
                return {};
            }

            return &typedResource->data;
        }

        void Clear() {
            _resources.clear();
            _allocator.Reset();
        }
    };
}