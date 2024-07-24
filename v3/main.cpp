// This has been adapted from the Vulkan tutorial

#include "modules/Starter.hpp"
#include "modules/TextMaker.hpp"


// Uniform struct
struct UniformBufferObject {
	alignas(16) glm::vec3 cameraPos;
	alignas(16) glm::mat4 invViewMatrix;
	alignas(16) glm::mat4 invProjectionMatrix;
};

struct GlobalUniformBufferObject {
	alignas(16) int numberOfSamples;
	alignas(16) int width;
	alignas(16) int height;
};


// Vertex struct
struct Vertex {
	glm::vec3 pos;
};



// MAIN  
class E09 : public BaseProject {
protected:
	//-----------------------------------------------------------
	//------------------------ VARIABLES ------------------------
	//-----------------------------------------------------------

	// Descriptor Layouts ["classes" of what will be passed to the shaders]
	DescriptorSetLayout DSLglobal;	// For Global values
	DescriptorSetLayout DSLray;	// For Blinn Objects

	// Vertex formats
	VertexDescriptor VD;

	// Pipelines [Shader couples]
	Pipeline Pray;

	// Scenes and texts

	// Models, textures and Descriptor Sets (values assigned to the uniforms)
	Model Mtri;
	Texture Timage;


	DescriptorSet DSray, DSGlobal; // Even if we have just one object, since we have two DSL, we also need two sets.

	int numberOfSamples = -1; //così diventa 0 e non do peso a quello salvato all'inizio nella texture

	// Other application parameters
	int currScene = 0;
	int subpass = 0;

	glm::vec3 CamPos = glm::vec3(-10.0, 5, 7.0);
	float CamAlpha = glm::radians(-75.0f);
	float CamBeta = glm::radians(-10.0f);
	float Ar;


	//-----------------------------------------------------------
	//------------------------- METHODS -------------------------
	//-----------------------------------------------------------

	/* Main application parameters */
	void setWindowParameters() {
		windowWidth = 800;
		windowHeight = 600;
		windowTitle = "GraphicsProject";
		windowResizable = GLFW_TRUE;
		initialBackgroundColor = { 0.1f, 0.1f, 0.1f, 1.0f };

		Ar = (float)windowWidth / (float)windowHeight;
	}

	/* Window resize */
	void onWindowResize(int w, int h) {
		std::cout << "Window resized to: " << w << " x " << h << "\n";
		Ar = (float)w / (float)h;
	}

	/* Load and setup all your Vulkan Models and Texutures. Create your Descriptor set layouts and load the shaders for the pipelines */
	void localInit() {
		// Descriptor Layouts [what will be passed to the shaders]
		DSLglobal.init(this, {
					{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS, sizeof(GlobalUniformBufferObject), 1},
					{1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0, 1}
			});
		DSLray.init(this, {
					{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS, sizeof(UniformBufferObject), 1},
			});


		// Vertex descriptors
		VD.init(this, {
				  {0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX}
			}, {
			  {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos), sizeof(glm::vec3), POSITION}
			});



		// Pipelines [Shader couples]
		Pray.init(this, &VD, "shaders/RayVert.spv", "shaders/RayFrag.spv", { &DSLglobal, &DSLray });
		Pray.setAdvancedFeatures(VK_COMPARE_OP_LESS, VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, false);

		// Models

		//cover the screen with 2 triangles
		std::vector<Vertex> quadVertices = {
			{{-1.0f,  1.0f, 0.0f}},  // Top-left
			{{-1.0f, -1.0f, 0.0f}},  // Bottom-left
			{{ 1.0f, -1.0f, 0.0f}},  // Bottom-right
			{{ 1.0f,  1.0f, 0.0f}}   // Top-right
		};

		std::vector<uint32_t> indices = {
			0, 1, 2,  // First triangle
			0, 2, 3   // Second triangle
		};

		Mtri.vertices = std::vector<unsigned char>(quadVertices.size() * sizeof(Vertex));
		memcpy(Mtri.vertices.data(), quadVertices.data(), Mtri.vertices.size());

		Mtri.indices = indices;
		Mtri.initMesh(this, &VD);


		// Textures
		Timage.init(this, "textures/texture.png");

		// Descriptor pool sizes
		// WARNING!!!!!!!!
		// Must be set before initializing the text and the scene
		DPSZs.uniformBlocksInPool = 2; //2.1.2
		DPSZs.texturesInPool = 1;
		DPSZs.setsInPool = 2;


		std::cout << "Initializing text\n";

		std::cout << "Initialization completed!\n";
		std::cout << "Uniform Blocks in the Pool  : " << DPSZs.uniformBlocksInPool << "\n";
		std::cout << "Textures in the Pool        : " << DPSZs.texturesInPool << "\n";
		std::cout << "Descriptor Sets in the Pool : " << DPSZs.setsInPool << "\n";
	}

	/* Create pipelines and Descriptor Sets */
	void pipelinesAndDescriptorSetsInit() {
		// Pipeline 
		Pray.create();


		// Define the data set
		DSray.init(this, &DSLray, { });
		DSGlobal.init(this, &DSLglobal, { &Timage });
	}

	Texture getImage() {
		return Timage;
	}

	/* Destroy pipelines and Descriptor Sets */
	void pipelinesAndDescriptorSetsCleanup() {
		// Cleanup pipelines
		Pray.cleanup();


		// Cleanup Descriptor Sets
		DSray.cleanup();
		DSGlobal.cleanup();
	}

	/* Here you destroy all the Models, Texture, Desc. Set Layouts and Pipelines */
	void localCleanup() {
		// Cleanup Models and Textures
		Mtri.cleanup();
		Timage.cleanup();

		// Cleanup Descriptor Set Layouts
		DSLray.cleanup();
		DSLglobal.cleanup();


		// Destroy Pipelines
		Pray.destroy();

	}


	/* Creation of the command buffer: send to the GPU all the objects you want to draw, with their buffers and textures */
	void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) {
		/* for each object:
			- bind the pipeline
			- bind the models
			- bind the descriptor sets
			- draw call
		*/
		Pray.bind(commandBuffer);
		Mtri.bind(commandBuffer);
		DSGlobal.bind(commandBuffer, Pray, 0, currentImage);	// The Global Descriptor Set (Set 0)
		DSray.bind(commandBuffer, Pray, 1, currentImage);	// The Material and Position Descriptor Set (Set 1)
		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(Mtri.indices.size()), 1, 0, 0, 0);

	}


	/* Update uniforms */
	void updateUniformBuffer(uint32_t currentImage) {
		float deltaT;
		glm::vec3 m = glm::vec3(0.0f), r = glm::vec3(0.0f);
		bool fire = false;
		getSixAxis(deltaT, m, r, fire);

		if (m != glm::vec3(0.0f, 0.0f, 0.f) || r != glm::vec3(0.0f, 0.0f, 0.f)) {
			numberOfSamples = 0; //in questo caso ci serve = 0 almeno nella shader la media pesata non considera il previous frame (perchè ci stiamo muovendo)
			std::cout << "moving" << "\n";
		}

		static float autoTime = true;
		static float cTime = 0.0;
		const float turnTime = 36.0f;
		const float angTurnTimeFact = 2.0f * M_PI / turnTime;

		if (autoTime) {
			cTime = cTime + deltaT;
			cTime = (cTime > turnTime) ? (cTime - turnTime) : cTime;
		}
		cTime += r.z * angTurnTimeFact * 4.0;

		const float ROT_SPEED = glm::radians(120.0f);
		const float MOVE_SPEED = 2.0f;

		CamAlpha = CamAlpha - ROT_SPEED * deltaT * r.y;
		CamBeta = CamBeta - ROT_SPEED * deltaT * r.x;
		CamBeta = CamBeta < glm::radians(-90.0f) ? glm::radians(-90.0f) :
			(CamBeta > glm::radians(90.0f) ? glm::radians(90.0f) : CamBeta);

		glm::vec3 ux = glm::rotate(glm::mat4(1.0f), CamAlpha, glm::vec3(0, 1, 0)) * glm::vec4(1, 0, 0, 1);
		glm::vec3 uz = glm::rotate(glm::mat4(1.0f), CamAlpha, glm::vec3(0, 1, 0)) * glm::vec4(0, 0, 1, 1);
		CamPos = CamPos + MOVE_SPEED * m.x * ux * deltaT;
		CamPos = CamPos + MOVE_SPEED * m.y * glm::vec3(0, 1, 0) * deltaT;
		CamPos = CamPos + MOVE_SPEED * m.z * uz * deltaT;

		static float subpassTimer = 0.0;


		// Standard procedure to quit when the ESC key is pressed
		if (glfwGetKey(window, GLFW_KEY_ESCAPE)) {
			glfwSetWindowShouldClose(window, GL_TRUE);
		}


		// Here is where you actually update your uniforms
		glm::mat4 M = glm::perspective(glm::radians(45.0f), Ar, 0.1f, 50.0f);
		M[1][1] *= -1;

		glm::mat4 Mv = glm::rotate(glm::mat4(1.0), -CamBeta, glm::vec3(1, 0, 0)) *
			glm::rotate(glm::mat4(1.0), -CamAlpha, glm::vec3(0, 1, 0)) *
			glm::translate(glm::mat4(1.0), -CamPos);

		glm::mat4 ViewPrj = M * Mv;
		glm::mat4 baseTr = glm::mat4(1.0f);

		// updates global uniforms
		// Global
		GlobalUniformBufferObject gubo{};
		gubo.numberOfSamples = numberOfSamples;
		gubo.height = windowHeight;
		gubo.width = windowWidth;
		DSGlobal.map(currentImage, &gubo, 0);
		numberOfSamples += 1;
		//std::cout << numberOfSamples << "\n";

		// objects
		UniformBufferObject ubo{};
		ubo.cameraPos = CamPos;
		ubo.invViewMatrix = glm::inverse(Mv);
		ubo.invProjectionMatrix = glm::inverse(M);

		DSray.map(currentImage, &ubo, 0);
	}
};

// This is the main: probably you do not need to touch this!
int main() {
	E09 app;

	try {
		app.run();
	}
	catch (const std::exception & e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}