#include "KayUtils.h"

Material* KayUtils::CreateDefaultMaterial()
{
	Material *material = Material::create("res/shaders/colored.vert", "res/shaders/colored.frag", "");
	material->setParameterAutoBinding("u_worldViewProjectionMatrix", "WORLD_VIEW_PROJECTION_MATRIX");
	material->getParameter("u_diffuseColor")->setValue(Vector4(1.0, 1.0, 1.0, 0.3f));
	material->getStateBlock()->setCullFace(false);
	material->getStateBlock()->setDepthTest(true);
	material->getStateBlock()->setDepthWrite(true);
	material->getStateBlock()->setBlend(true);
	material->getStateBlock()->setBlendSrc(RenderState::BLEND_SRC_ALPHA);
	material->getStateBlock()->setBlendDst(RenderState::BLEND_ONE_MINUS_SRC_ALPHA);
	return material;
}

Model* KayUtils::CreateGridModel(unsigned int rows)
{
	const float y = 0.0f;
	const float x1 = -25000.0f;
	const float z1 = -25000.0f;
	const float x2 = 25000.0f;
	const float z2 = 25000.0f;
	float xDelta = (x2 - x1) / rows;
	float zDelta = (z2 - z1) / rows;
	unsigned int pointCount = rows * rows * 2;
	Vector3 *points = (Vector3 *)malloc(sizeof(Vector3) * pointCount);
	unsigned int index = 0;
	for (float r = x1; r < x2; r += xDelta)
	{
		points[index++] = Vector3(r, y, z1);
		points[index++] = Vector3(r, y, z2);
	}
	for (float c = z1; c < z2; c += zDelta)
	{
		points[index++] = Vector3(x1, y, c);
		points[index++] = Vector3(x2, y, c);
	}
	Mesh *mesh = Mesh::createLines(points, pointCount, true);
	free(points);
	Model *gridModel = Model::create(mesh);
	Material *material = CreateDefaultMaterial();
	gridModel->setMaterial(material);
	SAFE_RELEASE(material);
	return gridModel;
}