#include "LauCommon.h"
#include "Game.hpp"
#include "Factories.hpp"
#include "utils/IO.h"
#include "GameObject.hpp"

#include "MeshRenderer.hpp"
#include "Mesh.hpp"
#include "Camera.hpp"

#include "Light.hpp"

#include "math/Matrix.hpp"
#include "math/Vector.hpp"

using namespace std;
using namespace lau::math;

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

void MeshRenderer::start() {
    const int DIMS = 3;
	auto mesh = gameObject->getComponent<Mesh>();
    if(mesh->isLoaded()) {
        vbo.create(DIMS, mesh->vertices, mesh->normals, mesh->faces);
        vbo.bindAttributes(shader);
    } else {
        mesh->onLoad.subscribe([this]() {
          Game::scheduleMainThreadTask([this]() {
            auto mesh = gameObject->getComponent<Mesh>();
            vbo.create(DIMS, mesh->vertices, mesh->normals, mesh->faces);
            vbo.bindAttributes(shader);
          });
        });
    }
}

void MeshRenderer::draw(float alpha) {
	auto mesh = gameObject->getComponent<Mesh>();
    if(mesh == nullptr || !mesh->isLoaded())
        return;

    auto& transform = gameObject->transform;
    auto camera = Camera::current;
    vbo.bindForDrawing(shader);
    // TODO check out with which frequency I need to update uniforms -- are they replaced? are they stored in a local memory obj? whats their lifetime?
    shader.uniformMatrix4fv(shader.projectionUniformLocation, 1, camera->projection.data);
    shader.uniformMatrix4fv(shader.world2cameraUniformLocation, 1, camera->world2camera.data);
    const mat4& object2world = transform.getObject2WorldMatrix();
    shader.uniformMatrix4fv(shader.object2worldUniformLocation, 1, object2world.data);
    const mat4& object2world_it = transform.getObject2WorldTranspOfInvMatrix();
    shader.uniformMatrix4fv(shader.object2worldITUniformLocation, 1, object2world_it.data);

    // Lights
    auto& lights = Light::allLights();
    auto lightPositions = Light::allLightPositions();
    auto lightColors = Light::allLightColors();
    shader.uniform1i(shader.numLightsUniformLocation, lights.size());
    shader.uniform3fv(shader.lightPositionsUniformLocation, lights.size(), lightPositions.data());
    shader.uniform4fv(shader.lightColorsUniformLocation, lights.size(), lightColors.data());

    glDrawElements(GL_TRIANGLES, vbo.primitivesCount(), GL_UNSIGNED_INT, 0);
}

/////
// Factory
#define MESH_RENDERER_ID 2
__LAU_CREATE_COMPONENT_INITIALIZER(lau::MeshRenderer, MESH_RENDERER_ID)

} // namespace lau
