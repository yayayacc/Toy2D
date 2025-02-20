#include "runtime/resource/resource.h"

namespace Toy2D {
    IResource::IResource(std::string_view name) :
        m_name(name) {
    }

    std::string_view IResource::getName() const { return m_name; }
    size_t           IResource::getUUID() const { return Hash(m_name); }

    // ****************************************************************************

    template class Resource<ResourceType::Mesh>;
    template class Resource<ResourceType::Texture>;
    template class Resource<ResourceType::Animation>;
} // namespace Toy2D
