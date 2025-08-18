///////////////////////////////////////////////////////////////////////////////
// scenemanager.cpp
// ============
// manage the preparing and rendering of 3D scenes - textures, materials, lighting
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//	Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
///////////////////////////////////////////////////////////////////////////////

#include "SceneManager.h"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif

#include <glm/gtx/transform.hpp>

// declaration of global variables
namespace
{
	const char* g_ModelName = "model";
	const char* g_ColorValueName = "objectColor";
	const char* g_TextureValueName = "objectTexture";
	const char* g_UseTextureName = "bUseTexture";
	const char* g_UseLightingName = "bUseLighting";
}

/***********************************************************
 *  SceneManager()
 *
 *  The constructor for the class
 ***********************************************************/
SceneManager::SceneManager(ShaderManager *pShaderManager)
{
	m_pShaderManager = pShaderManager;
	m_basicMeshes = new ShapeMeshes();
}

/***********************************************************
 *  ~SceneManager()
 *
 *  The destructor for the class
 ***********************************************************/
SceneManager::~SceneManager()
{
	m_pShaderManager = NULL;
	delete m_basicMeshes;
	m_basicMeshes = NULL;
}

/***********************************************************
 *  CreateGLTexture()
 *
 *  This method is used for loading textures from image files,
 *  configuring the texture mapping parameters in OpenGL,
 *  generating the mipmaps, and loading the read texture into
 *  the next available texture slot in memory.
 ***********************************************************/
bool SceneManager::CreateGLTexture(const char* filename, std::string tag)
{
	int width = 0;
	int height = 0;
	int colorChannels = 0;
	GLuint textureID = 0;

	// indicate to always flip images vertically when loaded
	stbi_set_flip_vertically_on_load(true);

	// try to parse the image data from the specified image file
	unsigned char* image = stbi_load(
		filename,
		&width,
		&height,
		&colorChannels,
		0);

	// if the image was successfully read from the image file
	if (image)
	{
		std::cout << "Successfully loaded image:" << filename << ", width:" << width << ", height:" << height << ", channels:" << colorChannels << std::endl;

		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);

		// set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// if the loaded image is in RGB format
		if (colorChannels == 3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		// if the loaded image is in RGBA format - it supports transparency
		else if (colorChannels == 4)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		else
		{
			std::cout << "Not implemented to handle image with " << colorChannels << " channels" << std::endl;
			return false;
		}

		// generate the texture mipmaps for mapping textures to lower resolutions
		glGenerateMipmap(GL_TEXTURE_2D);

		// free the image data from local memory
		stbi_image_free(image);
		glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

		// register the loaded texture and associate it with the special tag string
		m_textureIDs[m_loadedTextures].ID = textureID;
		m_textureIDs[m_loadedTextures].tag = tag;
		m_loadedTextures++;
		m_textureIDs.push_back(textureID);
		return true;
	}

	std::cout << "Could not load image:" << filename << std::endl;

	// Error loading the image
	return false;
}

/***********************************************************
 *  BindGLTextures()
 *
 *  This method is used for binding the loaded textures to
 *  OpenGL texture memory slots.  There are up to 16 slots.
 ***********************************************************/
void SceneManager::BindGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		// bind textures on corresponding texture units
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, m_textureIDs[i].ID);
	}
}

// old

/***********************************************************
 *  DestroyGLTextures()
 *
 *  This method is used for freeing the memory in all the
 *  used texture memory slots.
 ***********************************************************/
/* void SceneManager::DestroyGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		glGenTextures(1, &m_textureIDs[i].ID);
	}
}

*/

// new

void SceneManager::DeleteSceneTextures()
{
	if (!m_textureIDs.empty()) {
		glDeleteTextures(static_cast<GLsizei>(m_textureIDs.size()), m_textureIDs.data());
		std::cout << "[SceneManager] Deleted " << m_textureIDs.size() << " texture(s)." << std::endl;
		m_textureIDs.clear();  // Clear the list after deletion
	} else {
		std::cout << "[SceneManager] No textures to delete." << std::endl;
	}
}



/***********************************************************
 *  FindTextureID()
 *
 *  This method is used for getting an ID for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureID(std::string tag)
{
	int textureID = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureID = m_textureIDs[index].ID;
			bFound = true;
		}
		else
			index++;
	}

	return(textureID);
}

/***********************************************************
 *  FindTextureSlot()
 *
 *  This method is used for getting a slot index for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureSlot(std::string tag)
{
	int textureSlot = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureSlot = index;
			bFound = true;
		}
		else
			index++;
	}

	return(textureSlot);
}

/***********************************************************
 *  FindMaterial()
 *
 *  This method is used for getting a material from the previously
 *  defined materials list that is associated with the passed in tag.
 ***********************************************************/
bool SceneManager::FindMaterial(std::string tag, OBJECT_MATERIAL& material)
{
	if (m_objectMaterials.size() == 0)
	{
		return(false);
	}

	int index = 0;
	bool bFound = false;
	while ((index < m_objectMaterials.size()) && (bFound == false))
	{
		if (m_objectMaterials[index].tag.compare(tag) == 0)
		{
			bFound = true;
			material.diffuseColor = m_objectMaterials[index].diffuseColor;
			material.specularColor = m_objectMaterials[index].specularColor;
			material.shininess = m_objectMaterials[index].shininess;
		}
		else
		{
			index++;
		}
	}

	return(true);
}

/***********************************************************
 *  SetTransformations()
 *
 *  This method is used for setting the transform buffer
 *  using the passed in transformation values.
 ***********************************************************/
void SceneManager::SetTransformations(
	glm::vec3 scaleXYZ,
	float XrotationDegrees,
	float YrotationDegrees,
	float ZrotationDegrees,
	glm::vec3 positionXYZ,
	glm::vec3 offset)
{
	// variables for this method
	glm::mat4 modelView;
	glm::mat4 scale;
	glm::mat4 rotationX;
	glm::mat4 rotationY;
	glm::mat4 rotationZ;
	glm::mat4 translation;

	// set the scale value in the transform buffer
	scale = glm::scale(scaleXYZ);
	// set the rotation values in the transform buffer
	rotationX = glm::rotate(glm::radians(XrotationDegrees), glm::vec3(1.0f, 0.0f, 0.0f));
	rotationY = glm::rotate(glm::radians(YrotationDegrees), glm::vec3(0.0f, 1.0f, 0.0f));
	rotationZ = glm::rotate(glm::radians(ZrotationDegrees), glm::vec3(0.0f, 0.0f, 1.0f));
	// set the translation value in the transform buffer
	translation = glm::translate(positionXYZ + offset);

	modelView = translation * rotationZ * rotationY * rotationX * scale;

	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setMat4Value(g_ModelName, modelView);
	}
}

/***********************************************************
 *  SetShaderColor()
 *
 *  This method is used for setting the passed in color
 *  into the shader for the next draw command
 ***********************************************************/
void SceneManager::SetShaderColor(
	float redColorValue,
	float greenColorValue,
	float blueColorValue,
	float alphaValue)
{
	// variables for this method
	glm::vec4 currentColor;

	currentColor.r = redColorValue;
	currentColor.g = greenColorValue;
	currentColor.b = blueColorValue;
	currentColor.a = alphaValue;

	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setIntValue(g_UseTextureName, false);
		m_pShaderManager->setVec4Value(g_ColorValueName, currentColor);
	}
}

/***********************************************************
 *  SetShaderTexture()
 *
 *  This method is used for setting the texture data
 *  associated with the passed in ID into the shader.
 ***********************************************************/
void SceneManager::SetShaderTexture(
	std::string textureTag)
{
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setIntValue(g_UseTextureName, true);

		int textureID = -1;
		textureID = FindTextureSlot(textureTag);
		m_pShaderManager->setSampler2DValue(g_TextureValueName, textureID);
	}
}

/***********************************************************
 *  SetTextureUVScale()
 *
 *  This method is used for setting the texture UV scale
 *  values into the shader.
 ***********************************************************/
void SceneManager::SetTextureUVScale(float u, float v)
{
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setVec2Value("UVscale", glm::vec2(u, v));
	}
}

/***********************************************************
 *  SetShaderMaterial()
 *
 *  This method is used for passing the material values
 *  into the shader.
 ***********************************************************/
void SceneManager::SetShaderMaterial(
	std::string materialTag)
{
	if (m_objectMaterials.size() > 0)
	{
		OBJECT_MATERIAL material;
		bool bReturn = false;

		bReturn = FindMaterial(materialTag, material);
		if (bReturn == true)
		{
			m_pShaderManager->setVec3Value("material.diffuseColor", material.diffuseColor);
			m_pShaderManager->setVec3Value("material.specularColor", material.specularColor);
			m_pShaderManager->setFloatValue("material.shininess", material.shininess);
		}
	}
}

/**************************************************************/
/*** STUDENTS CAN MODIFY the code in the methods BELOW for  ***/
/*** preparing and rendering their own 3D replicated scenes.***/
/*** Please refer to the code in the OpenGL sample project  ***/
/*** for assistance.                                        ***/
/**************************************************************/


/***********************************************************
 *  PrepareScene()
 *
 *  This method is used for preparing the 3D scene by loading
 *  the shapes, textures in memory to support the 3D scene 
 *  rendering
 ***********************************************************/
//-------------------------------------------------------------------------------------------------------textures

void SceneManager::DefineObjectMaterials()
{
	OBJECT_MATERIAL goldMaterial;
	goldMaterial.diffuseColor = glm::vec3(0.4f, 0.4f, 0.4f);
	goldMaterial.specularColor = glm::vec3(0.7f, 0.7f, 0.6f);
	goldMaterial.shininess = 52.0;
	goldMaterial.tag = "metal";

	m_objectMaterials.push_back(goldMaterial);

	OBJECT_MATERIAL woodMaterial;
	woodMaterial.diffuseColor = glm::vec3(0.2f, 0.2f, 0.3f);
	woodMaterial.specularColor = glm::vec3(0.0f, 0.0f, 0.0f);
	woodMaterial.shininess = 0.1;
	woodMaterial.tag = "wood";

	m_objectMaterials.push_back(woodMaterial);

	OBJECT_MATERIAL glassMaterial;
	glassMaterial.diffuseColor = glm::vec3(0.2f, 0.2f, 0.2f);
	glassMaterial.specularColor = glm::vec3(1.0f, 1.0f, 1.0f);
	glassMaterial.shininess = 95.0;
	glassMaterial.tag = "glass";

}


void SceneManager::SetupSceneLights()
{
	// this line of code is NEEDED for telling the shaders to render 
	// the 3D scene with custom lighting - to use the default rendered 
	// lighting then comment out the following line
	m_pShaderManager->setBoolValue(g_UseLightingName, true);


	//----------------------------------------------------------------------------------side fan
	m_pShaderManager->setVec3Value("pointLights[0].position", -3.0f, 8.5f, 2.0f);
	m_pShaderManager->setVec3Value("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
	m_pShaderManager->setVec3Value("pointLights[0].diffuse", 0.0f, 2.6f, 3.0f);
	m_pShaderManager->setVec3Value("pointLights[0].specular", 0.0f, 2.6f, 3.0f);
	m_pShaderManager->setBoolValue("pointLights[0].bActive", true);
	//---------------------------------------------------------------------------------back right middle fan
	m_pShaderManager->setVec3Value("pointLights[1].position", 0.5f, 0.2f, 1.5f);
	m_pShaderManager->setVec3Value("pointLights[1].ambient", 0.05f, 0.05f, 0.05f);
	m_pShaderManager->setVec3Value("pointLights[1].diffuse", 0.0f, 2.6f, 3.0f);
	m_pShaderManager->setVec3Value("pointLights[1].specular", 0.0f, 0.9f, 1.0f);
	m_pShaderManager->setBoolValue("pointLights[1].bActive", true);
	//----------------------------------------------------------------------------------bottom middle fan
	m_pShaderManager->setVec3Value("pointLights[2].position", 0.5f, 1.2f, 0.5f);
	m_pShaderManager->setVec3Value("pointLights[2].ambient", 0.05f, 0.05f, 0.05f);
	m_pShaderManager->setVec3Value("pointLights[2].diffuse", 0.0f, 2.6f, 3.0f);
	m_pShaderManager->setVec3Value("pointLights[2].specular", 0.0f, 0.9f, 1.0f);
	m_pShaderManager->setBoolValue("pointLights[2].bActive", true);
    //-----------------------------------------------------------------------------------room
	m_pShaderManager->setVec3Value("pointLights[3].position", 10.1f, 30.0f, 30.0f);
	m_pShaderManager->setVec3Value("pointLights[3].ambient", 0.3f, 0.3f, 0.3f);
	m_pShaderManager->setVec3Value("pointLights[3].diffuse", 0.2f, 0.2f, 0.2f);
	m_pShaderManager->setVec3Value("pointLights[3].specular", 3.9f, 3.9f, 3.9f);
	m_pShaderManager->setBoolValue("pointLights[3].bActive", true);

	//m_pShaderManager->setVec3Value("pointLights[1].position", -7.0f, 8.5f, 2.0f);
	//m_pShaderManager->setVec3Value("pointLights[1].ambient", 0.0f, 0.9f, 0.6f);
	//m_pShaderManager->setVec3Value("pointLights[1].diffuse", 0.2f, 0.2f, 1.0f);
	//m_pShaderManager->setVec3Value("pointLights[1].specular", 0.9f, 0.9f, 0.9f);
	//m_pShaderManager->setBoolValue("pointLights[1].bActive", true);
	//10.1f, 30.0f, 30.0f

}

void SceneManager::LoadSceneTextures()
{
	bool bReturn = false;

	// [ERROR HANDLING ADDED] Log any texture that fails to load
	auto tryLoadTexture = [&](const std::string& path, const std::string& tag) {
		bReturn = CreateGLTexture(path.c_str(), tag.c_str());
		if (!bReturn) {
			std::cerr << "[SceneManager] ERROR: Failed to load texture: " << path << std::endl;
		}
		};

	tryLoadTexture("textures/desk.jpg", "desk");
	tryLoadTexture("textures/wall.jpg", "wall");
	tryLoadTexture("textures/keyboard.jpg", "keyboard");
	tryLoadTexture("textures/black.jpg", "black");
	tryLoadTexture("textures/screen.jpg", "screen");
	tryLoadTexture("textures/rightscreen.jpg", "rightscreen");
	tryLoadTexture("textures/leftscreen.jpg", "leftscreen");
	tryLoadTexture("textures/cpucooler.jpg", "cpucooler");
	tryLoadTexture("textures/gpufront.jpg", "gpufront");
	tryLoadTexture("textures/gpuside.jpg", "gpuside");
	tryLoadTexture("textures/gputop.jpg", "gputop");
	tryLoadTexture("textures/motherboard.jpg", "motherboard");
	tryLoadTexture("textures/ram.jpg", "ram");

	BindGLTextures();
}







void SceneManager::PrepareScene()
{
	// only one instance of a particular mesh needs to be
	// loaded in memory no matter how many times it is drawn
	// in the rendered 3D scene
	LoadSceneTextures();
	m_basicMeshes->LoadPlaneMesh();
	m_basicMeshes->LoadBoxMesh();
	m_basicMeshes->LoadTorusMesh();
	m_basicMeshes->LoadPyramid4Mesh();
	m_basicMeshes->LoadPrismMesh();
	m_basicMeshes->LoadCylinderMesh();

	SetupSceneLights();
	DefineObjectMaterials();

}

/***********************************************************
 *  RenderScene()
 *
 *  This method is used for rendering the 3D scene by 
 *  transforming and drawing the basic 3D shapes
 ***********************************************************/
void SceneManager::RenderScene()
{
	// declare the variables for the transformations
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;

	/*** Set needed transformations before drawing the basic mesh.  ***/
	/*** This same ordering of code should be used for transforming ***/
	/*** and drawing all the basic 3D shapes.						***/
	/******************************************************************/
	// set the XYZ scale for the mesh
	glm::mat4 scale;
	glm::mat4 rotation;
	glm::mat4 rotation2;
	glm::mat4 translation;
	glm::mat4 model;

	//------------------------------------------------------------------------------------------------Desk

	scaleXYZ = glm::vec3(30.0f, 1.0f, 10.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(0.0f, 0.0f, 0.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(0.8, 0.8, 0.8, 1);
	SetShaderTexture("desk");
	SetShaderMaterial("wood");
	// draw the mesh with transformation values
	m_basicMeshes->DrawPlaneMesh();
	/****************************************************************/

	scaleXYZ = glm::vec3(20.0f, 1.0f, 10.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 90.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-20.0f, 0.0f, 30.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(0.8, 0.8, 0.8, 1);
	SetShaderTexture("desk");
	SetShaderMaterial("wood");
	// draw the mesh with transformation values
	m_basicMeshes->DrawPlaneMesh();
	/****************************************************************/

	//------------------------------------------------------------------------------------------------Wall

	scaleXYZ = glm::vec3(30.0f, 1.0f, 10.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 90.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(0.0f, 10.0f, -10.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(0.5, 0.5, 0.5, 1);
	SetShaderTexture("wall");
	SetShaderMaterial("wood");
	// draw the mesh with transformation values
	m_basicMeshes->DrawPlaneMesh();

	scaleXYZ = glm::vec3(30.0f, 1.0f, 10.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 90.0f;
	YrotationDegrees = 90.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-30.0f, 10.0f, 20.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(0.5, 0.5, 0.5, 1);
	SetShaderTexture("wall");
	SetShaderMaterial("wood");
	// draw the mesh with transformation values
	m_basicMeshes->DrawPlaneMesh();
	//------------------------------------------------------------------------------------------------Keyboard

	scaleXYZ = glm::vec3(8.0f, 2.0f, 20.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 180.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-18.0f, -0.1f, 30.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(0.3, 0.3, 0.3, 1);
	SetShaderTexture("keyboard");
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::BoxSide::top);
	SetShaderTexture("black");
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::BoxSide::bottom);
	SetShaderTexture("black");
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::BoxSide::left);
	SetShaderTexture("black");
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::BoxSide::right);
	SetShaderTexture("black");
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::BoxSide::front);
	SetShaderTexture("black");
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::BoxSide::back);
	// draw the mesh with transformation values
	SetShaderMaterial("metal");
	m_basicMeshes->DrawBoxMesh();


	//------------------------------------------------------------------------------------------------Mouse Pad

	scaleXYZ = glm::vec3(5.0f, 1.0f, 5.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-17.0f, -0.1f, 16.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(0.1, 0.1, 0.12, 1);

	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();

	//------------------------------------------------------------------------------------------------Mouse 
	scaleXYZ = glm::vec3(1.0f, 1.0f, 1.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 90.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-17.0f, 0.1f, 16.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(0.1, 0.1, 0.12, 1);

	// draw the mesh with transformation values
	m_basicMeshes->DrawCylinderMesh();


	scaleXYZ = glm::vec3(2.0f, 2.5f, 1.5f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 180.0f;
	YrotationDegrees = 90.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-18.0f, -0.2f, 16.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(0.1, 0.1, 0.12, 1);

	// draw the mesh with transformation values
	m_basicMeshes->DrawPrismMesh();

	//------------------------------------------------------------------------------------------------Monitor 1

	scaleXYZ = glm::vec3(5.0f, 1.0f, 10.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-27.0f, -0.1f, 30.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(0.1, 0.1, 0.12, 1);

	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();

	scaleXYZ = glm::vec3(1.0f, 7.0f, 1.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-27.0f, -0.1f, 30.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(0.1, 0.1, 0.12, 1);

	// draw the mesh with transformation values
	m_basicMeshes->DrawCylinderMesh();


	scaleXYZ = glm::vec3(20.0f, 1.0f, 10.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 90.0f;
	YrotationDegrees = 90.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-26.0f, 10.f, 30.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(0.2, 0.2, 0.2, 1);
	SetShaderTexture("screen");	
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::BoxSide::top);
	SetShaderTexture("black");
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::BoxSide::bottom);
	SetShaderTexture("black");
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::BoxSide::left);
	SetShaderTexture("black");
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::BoxSide::right);
	SetShaderTexture("black");
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::BoxSide::front);
	SetShaderTexture("black");
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::BoxSide::back);
	SetShaderMaterial("glass");
	
	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();


	//------------------------------------------------------------------------------------------------Monitor 2

	scaleXYZ = glm::vec3(4.0f, 1.0f, 4.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = -45.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-22.5f, -0.1f, 16.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(0.1, 0.1, 0.12, 1);
	SetShaderMaterial("metal");
	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();




	scaleXYZ = glm::vec3(.5f, 7.0f, .5f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-22.5f, -0.1f, 16.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(0.1, 0.1, 0.12, 1);
	SetShaderMaterial("metal");
	// draw the mesh with transformation values
	m_basicMeshes->DrawCylinderMesh();


	scaleXYZ = glm::vec3(10.0f, 1.0f, 18.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 90.0f;
	YrotationDegrees = 45.0f;
	ZrotationDegrees = 0.0f;
	

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-22.5f, 10.0f, 16.0f);


	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	
	SetShaderTexture("rightscreen");
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::BoxSide::top);
	SetShaderTexture("black");
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::BoxSide::bottom);
	SetShaderTexture("black");
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::BoxSide::left);
	SetShaderTexture("black");
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::BoxSide::right);
	SetShaderTexture("black");
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::BoxSide::front);
	SetShaderTexture("black");
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::BoxSide::back);
	SetShaderMaterial("glass");
	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();



	//------------------------------------------------------------------------------------------------Monitor 3

	scaleXYZ = glm::vec3(4.0f, 1.0f, 4.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = -45.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-22.5f, -0.1f, 43.9f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(0.1, 0.1, 0.12, 1);
	SetShaderMaterial("metal");
	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();



	scaleXYZ = glm::vec3(.5f, 7.0f, .5f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-22.5f, -0.1f, 43.9f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(0.1, 0.1, 0.12, 1);
	SetShaderMaterial("metal");
	// draw the mesh with transformation values
	m_basicMeshes->DrawCylinderMesh();


	scaleXYZ = glm::vec3(10.0f, 1.0f, 18.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 90.0f;
	YrotationDegrees = 135.0f;
	ZrotationDegrees = 0.0f;


	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-22.5f, 10.0f, 43.9f);


	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);


	SetShaderTexture("leftscreen");
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::BoxSide::top);
	SetShaderTexture("black");
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::BoxSide::bottom);
	SetShaderTexture("black");
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::BoxSide::left);
	SetShaderTexture("black");
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::BoxSide::right);
	SetShaderTexture("black");
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::BoxSide::front);
	SetShaderTexture("black");
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::BoxSide::back);
	SetShaderMaterial("glass");
	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();















	//------------------------------------------------------------------------------------------------Left Side Panel 





	scaleXYZ = glm::vec3(10.0f, 1.0f, 12.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 90.0f;
	YrotationDegrees = 90.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-8.0f, 6.5f, 0.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(0.2, 0.2, 0.2, 1);
	SetShaderMaterial("metal");
	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();

	//SetShaderColor(1, 1, 1, 1);

	//m_basicMeshes->DrawBoxMeshLines();




	//------------------------------------------------------------------------------------------------Back Panel



	scaleXYZ = glm::vec3(19.0f, 1.0f, 11.99f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 90.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(1.0f, 6.5f, -5.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(0.2, 0.2, 0.2, 1);
	SetShaderMaterial("metal");
	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();

	//SetShaderColor(1, 1, 1, 1);

	//m_basicMeshes->DrawBoxMeshLines();



	//------------------------------------------------------------------------------------------------Bottom Panel

	scaleXYZ = glm::vec3(18.0f, 1.0f, 10.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(1.5f, 1.0f, 0.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(0.1, 0.1, 0.12, 1);
	SetShaderMaterial("metal");
	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();


	//SetShaderColor(1, 1, 1, 1);

	//m_basicMeshes->DrawBoxMeshLines();


	//------------------------------------------------------------------------------------------------Top Panel




	scaleXYZ = glm::vec3(18.0f, 1.0f, 10.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(1.5f, 12.0f, 0.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(0.1, 0.1, 0.12, 1);
	SetShaderMaterial("metal");
	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();


	//SetShaderColor(1, 1, 1, 1);

	//m_basicMeshes->DrawBoxMeshLines();


	//------------------------------------------------------------------------------------------------Front Left foot



	scaleXYZ = glm::vec3(1.0f, 0.5f, 1.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-8.0f, 0.25f, 4.5f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(0.2, 0.2, 0.2, 1);
	SetShaderMaterial("metal");
	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();


	//SetShaderColor(1, 1, 1, 1);

	//m_basicMeshes->DrawBoxMeshLines();



	//------------------------------------------------------------------------------------------------Front Right foot



	scaleXYZ = glm::vec3(1.0f, 0.5f, 1.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(10.0f, 0.25f, 4.5f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(0.2, 0.2, 0.2, 1);
	SetShaderMaterial("metal");
	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();


	//SetShaderColor(1, 1, 1, 1);

	//m_basicMeshes->DrawBoxMeshLines();



	//------------------------------------------------------------------------------------------------Back Right foot




	scaleXYZ = glm::vec3(1.0f, 0.5f, 1.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(10.0f, 0.25f, -5.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(0.2, 0.2, 0.2, 1);
	SetShaderMaterial("metal");
	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();


	//SetShaderColor(1, 1, 1, 1);

	//m_basicMeshes->DrawBoxMeshLines();


	//------------------------------------------------------------------------------------------------Back Left foot


	scaleXYZ = glm::vec3(1.0f, 0.5f, 1.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-8.0f, 0.25f, -5.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(0.2, 0.2, 0.2, 1);
	SetShaderMaterial("metal");
	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();


	//SetShaderColor(1, 1, 1, 1);

	//m_basicMeshes->DrawBoxMeshLines();



	//------------------------------------------------------------------------------------------------Motherboard Panel

	scaleXYZ = glm::vec3(13.0f, 3.5f, 11.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 90.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-1.0f, 6.5f, -3.7f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(0.4f, 0.4f, 0.4f, 1.0f);
	SetShaderMaterial("metal");
	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();

	//SetShaderColor(1, 1, 1, 1);

	//m_basicMeshes->DrawBoxMeshLines();


	//------------------------------------------------------------------------------------------------Motherboard

	scaleXYZ = glm::vec3(10.0f, 1.0f, 11.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 90.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-2.5f, 6.5f, -2.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(0.6f, 0.6f, 0.6f, 1.0f);
	SetShaderTexture("motherboard");
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::BoxSide::top);
	SetShaderTexture("black");
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::BoxSide::bottom);
	SetShaderTexture("black");
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::BoxSide::left);
	SetShaderTexture("black");
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::BoxSide::right);
	SetShaderTexture("black");
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::BoxSide::front);
	SetShaderTexture("black");
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::BoxSide::back);
	// draw the mesh with transformation values
	SetShaderMaterial("metal");
	m_basicMeshes->DrawBoxMesh();
	
	//SetShaderColor(1, 1, 1, 1);

	//m_basicMeshes->DrawBoxMeshLines();



	//------------------------------------------------------------------------------------------------Motherboard Side

	scaleXYZ = glm::vec3(3.0f, 1.5f, 5.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 90.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(4.0f, 9.0f, -1.2f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(0.2f, 0.2f, 0.2f, 1.0f);
	SetShaderMaterial("metal");
	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();

	//SetShaderColor(1, 1, 1, 1);

	//m_basicMeshes->DrawBoxMeshLines();

	//------------------------------------------------------------------



	scaleXYZ = glm::vec3(3.2f, 2.99f, 3.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 90.0f;
	ZrotationDegrees = -90.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(4.0f, 5.0f, -2.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(0.2f, 0.2f, 0.2f, 1.0f);
	SetShaderMaterial("metal");
	// draw the mesh with transformation values
	m_basicMeshes->DrawPrismMesh(); 

	//SetShaderColor(1, 1, 1, 1);

	//m_basicMeshes->DrawBoxMeshLines();

	//-------------------------------------------------------------------------------------------------Back Right Fans


	scaleXYZ = glm::vec3(4.0f, 1.0f, 11.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 90.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(8.0f, 6.5f, -4.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(0.1, 0.1, 0.12, 1);

	// draw the mesh with transformation values
	SetShaderMaterial("metal");
	m_basicMeshes->DrawBoxMesh();

	//SetShaderColor(1, 1, 1, 1);

	//m_basicMeshes->DrawBoxMeshLines();



	//--------------------------------------------------------------------------




	scaleXYZ = glm::vec3(1.5f, 1.5f, 1.5f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 90.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(8.0f, 6.5f, -3.5f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(0.0f, 0.9f, 1.0f, 1.0f);

	SetShaderMaterial("metal");
	// draw the mesh with transformation values
	m_basicMeshes->DrawTorusMesh();

	//SetShaderColor(1, 1, 1, 1);

	//m_basicMeshes->DrawBoxMeshLines();
	//--------------------------------------------------------------------------

	scaleXYZ = glm::vec3(1.5f, 1.5f, 1.5f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 90.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(8.0f, 3.3f, -3.5f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(0.0f, 0.9f, 1.0f, 1.0f);


	// draw the mesh with transformation values
	m_basicMeshes->DrawTorusMesh();

	//SetShaderColor(1, 1, 1, 1);

	//m_basicMeshes->DrawBoxMeshLines();
	//--------------------------------------------------------------------------
	scaleXYZ = glm::vec3(1.5f, 1.5f, 1.5f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 90.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(8.0f, 9.7f, -3.5f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(0.0f, 0.9f, 1.0f, 1.0f);


	// draw the mesh with transformation values
	m_basicMeshes->DrawTorusMesh();

	//SetShaderColor(1, 1, 1, 1);

	//m_basicMeshes->DrawBoxMeshLines();

	//--------------------------------------------------------------------------



	scaleXYZ = glm::vec3(0.5f, 0.2f, 0.5f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 90.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(8.0f, 9.7f, -3.5f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(0.0f, 0.0f, 0.0f, 1.0f);



	// draw the mesh with transformation values
	m_basicMeshes->DrawCylinderMesh();

	//SetShaderColor(1, 1, 1, 1);

	//m_basicMeshes->DrawBoxMeshLines();

	//--------------------------------------------------------------------------
	scaleXYZ = glm::vec3(0.5f, 0.2f, 0.5f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 90.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(8.0f, 3.3f, -3.5f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(0.0f, 0.0f, 0.0f, 1.0f);



	// draw the mesh with transformation values
	m_basicMeshes->DrawCylinderMesh();

	//SetShaderColor(1, 1, 1, 1);

	//m_basicMeshes->DrawBoxMeshLines();
	//--------------------------------------------------------------------------

	scaleXYZ = glm::vec3(0.5f, 0.2f, 0.5f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 90.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(8.0f, 6.5f, -3.5f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(0.0f, 0.0f, 0.0f, 1.0f);



	// draw the mesh with transformation values
	m_basicMeshes->DrawCylinderMesh();

	//SetShaderColor(1, 1, 1, 1);

	//m_basicMeshes->DrawBoxMeshLines();


	//--------------------------------------------------------------------------



	scaleXYZ = glm::vec3(1.2f, 0.2f, 1.2f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 90.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(8.0f, 6.5f, -3.6f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(0.3f, 0.95f, 1.0f, 1.0f);




	// draw the mesh with transformation values
	m_basicMeshes->DrawCylinderMesh();

	//SetShaderColor(1, 1, 1, 1);

	//m_basicMeshes->DrawBoxMeshLines();


	//--------------------------------------------------------------------------



	scaleXYZ = glm::vec3(1.2f, 0.2f, 1.2f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 90.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(8.0f, 3.3f, -3.6f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(0.3f, 0.95f, 1.0f, 1.0f);




	// draw the mesh with transformation values
	m_basicMeshes->DrawCylinderMesh();

	//SetShaderColor(1, 1, 1, 1);

	//m_basicMeshes->DrawBoxMeshLines();

//--------------------------------------------------------------------------


	scaleXYZ = glm::vec3(1.2f, 0.2f, 1.2f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 90.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(8.0f, 9.7f, -3.6f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(0.3f, 0.95f, 1.0f, 1.0f);




	// draw the mesh with transformation values
	m_basicMeshes->DrawCylinderMesh();

	//SetShaderColor(1, 1, 1, 1);

	//m_basicMeshes->DrawBoxMeshLines();



	//-----------------------------------------------------------------------------------------------------GPU


	scaleXYZ = glm::vec3(11.0f, 1.5f, 7.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-2.0f, 5.0f, 0.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(0.1, 0.1, 0.12, 1);
	
	SetShaderTexture("gputop");
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::BoxSide::top);
	SetShaderTexture("black");
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::BoxSide::bottom);
	SetShaderTexture("black");
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::BoxSide::left);
	SetShaderTexture("gpuside");
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::BoxSide::right);
	SetShaderTexture("gpufront");
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::BoxSide::front);
	SetShaderTexture("black");
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::BoxSide::back);
	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();


	//SetShaderColor(1, 1, 1, 1);

	//m_basicMeshes->DrawBoxMeshLines();


	

	

	//----------------------------------------------------------------------------------------------------Motherboard components 
	
	
	//--------------------------------------------------------------CPU Cooler

	scaleXYZ = glm::vec3(1.0f, 0.5f, 1.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 90.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 90.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-2.3f, 8.8f, -1.4f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(0.2f, 0.2f, 0.2f, 1.0f);
	SetShaderTexture("cpucooler");
	m_basicMeshes->DrawCylinderMesh(true, false, false);

	// draw the mesh with transformation values
	SetShaderTexture("black");
	m_basicMeshes->DrawCylinderMesh(false, false, true);



	//-----------------------------------------------------------------------------Ram


	scaleXYZ = glm::vec3(2.5f, 1.5f, 5.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 90.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(0.7f, 8.3f, -1.5f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(0.0f, 0.0f, 0.0f, 1.0f);
	SetShaderTexture("ram");
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::BoxSide::top);
	SetShaderTexture("black");
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::BoxSide::bottom);
	SetShaderTexture("black");
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::BoxSide::left);
	SetShaderTexture("black");
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::BoxSide::right);
	SetShaderTexture("black");
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::BoxSide::front);
	SetShaderTexture("black");
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::BoxSide::back);
	SetShaderMaterial("metal");


	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();

	//SetShaderColor(1, 1, 1, 1);

	//m_basicMeshes->DrawBoxMeshLines();



	//---------------------------------------------------------------------------------------------------Bottom Fans

	scaleXYZ = glm::vec3(4.0f, 1.0f, 11.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 90.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(1.5f, 1.5f, 2.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(0.1, 0.1, 0.12, 1);
	SetShaderMaterial("metal");
	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();

	//SetShaderColor(1, 1, 1, 1);

	//m_basicMeshes->DrawBoxMeshLines();


	//----------------------------------------------------------------------------

	scaleXYZ = glm::vec3(1.5f, 1.5f, 1.5f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 90.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(1.5f, 2.0f, 2.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(0.0f, 0.9f, 1.0f, 1.0f);

	SetShaderMaterial("metal");
	// draw the mesh with transformation values
	m_basicMeshes->DrawTorusMesh();

	//SetShaderColor(1, 1, 1, 1);

	//m_basicMeshes->DrawBoxMeshLines();

	//----------------------------------------------------------------------------

	scaleXYZ = glm::vec3(1.5f, 1.5f, 1.5f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 90.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-2.0f, 2.0f, 2.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(0.0f, 0.9f, 1.0f, 1.0f);

	SetShaderMaterial("metal");
	// draw the mesh with transformation values
	m_basicMeshes->DrawTorusMesh();

	//SetShaderColor(1, 1, 1, 1);

	//m_basicMeshes->DrawBoxMeshLines();



	//----------------------------------------------------------------------------

	scaleXYZ = glm::vec3(1.5f, 1.5f, 1.5f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 90.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(5.0f, 2.0f, 2.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(0.0f, 0.9f, 1.0f, 1.0f);

	SetShaderMaterial("metal");
	// draw the mesh with transformation values
	m_basicMeshes->DrawTorusMesh();

	//SetShaderColor(1, 1, 1, 1);

	//m_basicMeshes->DrawBoxMeshLines();





	//----------------------------------------------------------------------------
	scaleXYZ = glm::vec3(0.5f, 0.2f, 0.5f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 90.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(1.5f, 2.0f, 2.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(0.0f, 0.0f, 0.0f, 1.0f);
	SetShaderMaterial("metal");
	// draw the mesh with transformation values
	m_basicMeshes->DrawCylinderMesh();

	//SetShaderColor(1, 1, 1, 1);

	//m_basicMeshes->DrawBoxMeshLines();


	//----------------------------------------------------------------------------
	scaleXYZ = glm::vec3(0.5f, 0.2f, 0.5f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 90.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(5.0f, 2.0f, 2.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(0.0f, 0.0f, 0.0f, 1.0f);
	SetShaderMaterial("metal");
	// draw the mesh with transformation values
	m_basicMeshes->DrawCylinderMesh();

	//SetShaderColor(1, 1, 1, 1);

	//m_basicMeshes->DrawBoxMeshLines();


	//----------------------------------------------------------------------------
	scaleXYZ = glm::vec3(0.5f, 0.2f, 0.5f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 90.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-2.0f, 2.0f, 2.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(0.0f, 0.0f, 0.0f, 1.0f);
	SetShaderMaterial("metal");
	// draw the mesh with transformation values
	m_basicMeshes->DrawCylinderMesh();

	//SetShaderColor(1, 1, 1, 1);

	//m_basicMeshes->DrawBoxMeshLines();


	//------------------------------------------------------------------------------------------------

	scaleXYZ = glm::vec3(1.2f, 0.2f, 1.2f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 90.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(1.5f, 1.9f, 2.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(0.3f, 0.95f, 1.0f, 1.0f);


	SetShaderMaterial("metal");

	// draw the mesh with transformation values
	m_basicMeshes->DrawCylinderMesh();

	//SetShaderColor(1, 1, 1, 1);

	//m_basicMeshes->DrawBoxMeshLines();


	//------------------------------------------------------------------------------------------------

	scaleXYZ = glm::vec3(1.2f, 0.2f, 1.2f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 90.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(5.0f, 1.9f, 2.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(0.3f, 0.95f, 1.0f, 1.0f);

	SetShaderMaterial("metal");


	// draw the mesh with transformation values
	m_basicMeshes->DrawCylinderMesh();

	//SetShaderColor(1, 1, 1, 1);

	//m_basicMeshes->DrawBoxMeshLines();


	//------------------------------------------------------------------------------------------------

	scaleXYZ = glm::vec3(1.2f, 0.2f, 1.2f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 90.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-2.0f, 1.9f, 2.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(0.3f, 0.95f, 1.0f, 1.0f);

	SetShaderMaterial("metal");


	// draw the mesh with transformation values
	m_basicMeshes->DrawCylinderMesh();

	//SetShaderColor(1, 1, 1, 1);

	//m_basicMeshes->DrawBoxMeshLines();



	//------------------------------------------------------------------------------------------------------------------Side Fan





	scaleXYZ = glm::vec3(4.0f, 0.5f, 4.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 90.0f;
	YrotationDegrees = 90.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-7.5f, 8.5f, 2.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);


	SetShaderColor(0.1, 0.1, 0.12, 1);



	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();

	//SetShaderColor(1, 1, 1, 1);

	//m_basicMeshes->DrawBoxMeshLines();

	//-----------------------------------------------------------------------------------------


	scaleXYZ = glm::vec3(1.5f, 1.5f, 1.5f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 90.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-7.2f, 8.5f, 2.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(0.0f, 0.9f, 1.0f, 1.0f);


	// draw the mesh with transformation values
	m_basicMeshes->DrawTorusMesh();

	//SetShaderColor(1, 1, 1, 1);

	//m_basicMeshes->DrawBoxMeshLines();

	//-------------------------------------------------------------------------------------
	scaleXYZ = glm::vec3(0.5f, 0.2f, 0.5f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 90.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-7.0f, 8.5f, 2.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(0.0f, 0.0f, 0.0f, 1.0f);

	// draw the mesh with transformation values
	m_basicMeshes->DrawCylinderMesh();

	//SetShaderColor(1, 1, 1, 1);

	//m_basicMeshes->DrawBoxMeshLines();



	//------------------------------------------------------------------------------------------------

	
	scaleXYZ = glm::vec3(1.2f, 0.2f, 1.2f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 90.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-7.1f, 8.5f, 2.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	SetShaderColor(0.3f, 0.95f, 1.0f, 1.0f);




	// draw the mesh with transformation values
	m_basicMeshes->DrawCylinderMesh();

	//SetShaderColor(1, 1, 1, 1);

	//m_basicMeshes->DrawBoxMeshLines();




	//--------------------------------------------------------------------------------------------------------------Glass






	// Enable transparency
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Set glass transformations
	scaleXYZ = glm::vec3(18.0f, 0.1f, 9.99f);
	XrotationDegrees = 90.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(1.4f, 6.5f, 5.1f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// Disable depth writing for transparency
	glDepthMask(GL_FALSE);
	SetShaderMaterial("glass");
	// Set glass color (transparent light blue)
	SetShaderColor(0.6f, 0.8f, 1.0f, 0.2f);
	m_basicMeshes->DrawBoxMesh();

	// Restore depth writing
	glDepthMask(GL_TRUE);

	
	//------------------------------------------------------------------------------------------------



	// Enable transparency
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Set glass transformations
	scaleXYZ = glm::vec3(9.5f, 0.1f, 9.99f);
	XrotationDegrees = 90.0f;
	YrotationDegrees = 90.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(10.4f, 6.5f, 0.3f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// Disable depth writing for transparency
	glDepthMask(GL_FALSE);
	SetShaderMaterial("glass");
	// Set glass color (transparent light blue)
	SetShaderColor(0.6f, 0.8f, 1.0f, 0.2f);
	m_basicMeshes->DrawBoxMesh();

	// Restore depth writing
	glDepthMask(GL_TRUE);

}
void SceneManager::CleanupScene()
{
	DeleteSceneTextures();
	// Other cleanup logic...
}
