#include "LauCommon.h"
#include "Factories.hpp"
#include "utils/IO.h"
#include "GameObject.hpp"

#include "MeshRenderer.hpp"
#include "Mesh.hpp"
#include "Camera.hpp"

#include "Light.hpp"

using namespace std;
using namespace Eigen;

namespace lau {

MeshRenderer::MeshRenderer() {
    // Create shaders
    shader.loadShaders("default_assets/shaders/phong_interpolated_light.vs",
            "default_assets/shaders/phong_interpolated_light.fs");
}

MeshRenderer::MeshRenderer(const rapidjson::Value& fields) : MeshRenderer() {
}

void MeshRenderer::update(float dt) {
}

// TODO move this to the animation component
void MeshRenderer::draw(float alpha) {
    if(!shader.isReady()) return;

	auto mesh = gameObject->getComponent<Mesh>();
    auto& transform = gameObject->transform;
    auto camera = Camera::current;
	if(mesh != nullptr) {
		if(!wasInitialized && mesh->getVBO() != nullptr) {
			mesh->getVBO()->bindAttributes(shader);
			wasInitialized = true;
		}

		auto vbo = mesh->getVBO();
        if(vbo != nullptr) {
			vbo->bindForDrawing(shader);
            // TODO check out with which frequency I need to update uniforms -- are they replaced? are they stored in a local memory obj? whats their lifetime?
            shader.uniformMatrix4fv(shader.projectionUniformLocation, 1, camera->projection.data());
            shader.uniformMatrix4fv(shader.world2cameraUniformLocation, 1, camera->world2camera.data());
            const Matrix4f& object2world = transform.getObject2WorldMatrix();
            shader.uniformMatrix4fv(shader.object2worldUniformLocation, 1, object2world.data());
            const Matrix4f& object2world_it = transform.getObject2WorldTranspOfInvMatrix();
            shader.uniformMatrix4fv(shader.object2worldITUniformLocation, 1, object2world_it.data());

            // Lights
            auto& lights = Light::allLights();
            auto lightPositions = Light::allLightPositions();
            auto lightColors = Light::allLightColors();
            shader.uniform1i(shader.numLightsUniformLocation, lights.size());
            shader.uniform3fv(shader.lightPositionsUniformLocation, lights.size(), lightPositions.data());
            shader.uniform4fv(shader.lightColorsUniformLocation, lights.size(), lightColors.data());

            glDrawElements(GL_TRIANGLES, vbo->primitivesCount(), GL_UNSIGNED_INT, 0);
        }
	}
}

/////
// Factory
#define MESH_RENDERER_ID 2
__LAU_CREATE_COMPONENT_INITIALIZER(lau::MeshRenderer, MESH_RENDERER_ID)

} // namespace lau
