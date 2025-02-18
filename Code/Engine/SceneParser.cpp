#include "SceneParser.hpp"
#include <yaml-cpp/yaml.h>

namespace x {
    static void ParseWorld(const YAML::Node& world, SceneDescriptor& descriptor);
    static void ParseEntities(const YAML::Node& entities, SceneDescriptor& descriptor);
    static Float3 ParseFloat3(const YAML::Node& node, bool rgb = false);

    void SceneParser::Parse(const str& filename, SceneDescriptor& descriptor) {
        YAML::Node scene = YAML::LoadFile(filename);

        const auto name = scene["name"].as<str>();
        const auto desc = scene["description"].as<str>();

        descriptor.name        = name;
        descriptor.description = desc;

        ParseWorld(scene["world"], descriptor);
        ParseEntities(scene["entities"], descriptor);
    }

    void ParseWorld(const YAML::Node& world, SceneDescriptor& descriptor) {
        YAML::Node cameraNode = world["camera"];

        descriptor.world.camera.position = ParseFloat3(cameraNode["position"]);
        descriptor.world.camera.eye      = ParseFloat3(cameraNode["eye"]);
        descriptor.world.camera.fovY     = cameraNode["fovY"].as<f32>();
        descriptor.world.camera.nearZ    = cameraNode["nearZ"].as<f32>();
        descriptor.world.camera.farZ     = cameraNode["farZ"].as<f32>();

        YAML::Node lightNode = world["lights"];
        YAML::Node sunNode   = lightNode["sun"];

        descriptor.world.lights.sun.enabled      = sunNode["enabled"].as<bool>();
        descriptor.world.lights.sun.intensity    = sunNode["intensity"].as<f32>();
        descriptor.world.lights.sun.color        = ParseFloat3(sunNode["color"], true);
        descriptor.world.lights.sun.direction    = ParseFloat3(sunNode["direction"]);
        descriptor.world.lights.sun.castsShadows = sunNode["castsShadows"].as<bool>();
    }

    void ParseEntities(const YAML::Node& entities, SceneDescriptor& descriptor) {}

    Float3 ParseFloat3(const YAML::Node& node, const bool rgb) {
        const auto x = node[rgb ? "r" : "x"].as<f32>();
        const auto y = node[rgb ? "g" : "y"].as<f32>();
        const auto z = node[rgb ? "b" : "z"].as<f32>();

        return {x, y, z};
    }
}