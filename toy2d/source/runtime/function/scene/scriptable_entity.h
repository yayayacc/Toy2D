#pragma once

#include "runtime/function/scene/entity.h"

namespace Toy2D {
    class ScriptableEntity {
        friend class Scene;

    public:
        virtual ~ScriptableEntity() {}

        template <typename T>
        T& getComponent() {
            return m_entity.getComponent<T>();
        }

    protected:
        virtual void onCreate() {}
        virtual void onDestroy() {}
        virtual void onUpdate(TimeStep ts) {}

    private:
        Entity m_entity;
    };
} // namespace Toy2D