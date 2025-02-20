// Box2D
#include <box2d/b2_body.h>
#include <box2d/b2_fixture.h>
#include <box2d/b2_polygon_shape.h>
#include <box2d/b2_world.h>

#include "runtime/function/render/render_system/renderer_2d.h"
#include "runtime/function/scene/components.h"
#include "runtime/function/scene/entity.h"
#include "runtime/function/scene/scene.h"

namespace Toy2D {
    Scene::~Scene() {
    }

    Entity Scene::createEntity(const std::string& name) {
        static uint32_t create_entity_count = 0;

        Entity entity = {m_registry.create(), this};
        entity.addComponent<TransformComponent>();
        auto& name_component = entity.addComponent<NameComponent>();
        name_component.name  = (name.empty()) ? ("Entity" + std::to_string(create_entity_count)) :
                                                name;

        create_entity_count++;

        return entity;
    }

    void Scene::destroyEntity(Entity entity) {
        m_registry.destroy(entity);
    }

    void Scene::onRuntimeStart() {
        m_physics_world = new b2World({0.0f, -9.8f});
    }

    void Scene::onRuntimeStop() {
        delete m_physics_world;
        m_physics_world = nullptr;
    }

    void Scene::onUpdateRuntime(TimeStep timestep) {
        // update scripts
        {
            m_registry.view<NativeScriptComponent>().each([=](auto entity, auto& nsc) {
                if (!nsc.instance) {
                    nsc.instance           = nsc.instantiateScript();
                    nsc.instance->m_entity = Entity{entity, this};
                    nsc.instance->onCreate();
                }

                nsc.instance->onUpdate(timestep);
            });
        }

        // render
        SceneCamera* curr_camera = nullptr;
        Matrix       camera_transform{Matrix::Identity};
        {
            auto view = m_registry.view<TransformComponent, CameraComponent>();
            for (auto entity : view) {
                auto [transform, camera] = view.get<TransformComponent, CameraComponent>(entity);

                if (camera.is_current) {
                    curr_camera      = &camera.camera;
                    camera_transform = transform.getTransform();
                    break;
                }
            }
        }

        if (curr_camera) {
            Renderer2D::beginScene(*curr_camera, camera_transform);

            auto group = m_registry.group<TransformComponent>(entt::get<SpriteComponent>);
            for (auto entity : group) {
                auto [transform, sprite] = group.get<TransformComponent, SpriteComponent>(entity);

                Renderer2D::drawSprite(transform.getTransform(), sprite, (int)entity);
            }
        }

        Renderer2D::endScene();
    }

    void Scene::onUpdateEditor(TimeStep timestep, EditorCamera& camera) {
        Renderer2D::beginScene(camera);

        auto group = m_registry.group<TransformComponent>(entt::get<SpriteComponent>);
        for (auto entity : group) {
            auto [transform, sprite] = group.get<TransformComponent, SpriteComponent>(entity);

            Renderer2D::drawSprite(transform.getTransform(), sprite, (int)entity);
        }

        Renderer2D::endScene();
    }

    void Scene::onViewportResize(uint32_t width, uint32_t height) {
        m_viewport_width  = width;
        m_viewport_height = height;

        // resize non-fixedAspectRatio cameras
        auto view = m_registry.view<CameraComponent>();
        for (auto& entity : view) {
            auto& camera_component = view.get<CameraComponent>(entity);
            if (!camera_component.is_fixed_aspectRatio) {
                camera_component.camera.setViewportSize(width, height);
            }
        }
    }

    Entity Scene::getPrimaryCameraEntity() {
        auto view = m_registry.view<CameraComponent>();
        for (auto entity : view) {
            const auto& camera = view.get<CameraComponent>(entity);
            if (camera.is_current)
                return Entity{entity, this};
        }
        return {};
    }

    template <typename T>
    void Scene::onComponentAdded(Entity entity, T& component) {
        // static_assert(false);
    }

    template <>
    void Scene::onComponentAdded<TransformComponent>(Entity entity, TransformComponent& component) {
    }

    template <>
    void Scene::onComponentAdded<CameraComponent>(Entity entity, CameraComponent& component) {
        component.camera.setViewportSize(m_viewport_width, m_viewport_height);
    }

    template <>
    void Scene::onComponentAdded<SpriteComponent>(Entity entity, SpriteComponent& component) {
    }

    template <>
    void Scene::onComponentAdded<NameComponent>(Entity entity, NameComponent& component) {
    }

    template <>
    void Scene::onComponentAdded<NativeScriptComponent>(Entity entity, NativeScriptComponent& component) {
    }

} // namespace Toy2D