#pragma once
#include <nanogui/nanogui.h>
#include <nanogui/glutil.h>
#include <nanogui/screen.h>
#include <nanogui/layout.h>
#include <nanogui/window.h>
#include <nanogui/button.h>
#include <nanogui/entypo.h>
#include <nanogui/slider.h>
#include <nanogui/label.h>
#include "Renderer.hpp"

namespace Lykta {

	class Application : public nanogui::Screen {
	private:
		uint32_t texture = 0;
		std::unique_ptr<nanogui::GLShader> shader;
		std::unique_ptr<Renderer> renderer;
		nanogui::Window* window;
		nanogui::ComboBox* integratorBox;
		
	public:
		Application() : nanogui::Screen(Eigen::Vector2i(1024, 768), "lykta") {
			
			// Initialize renderer
			renderer = std::unique_ptr<Renderer>(new Renderer());
			
			// Initialize user interface
			initializeGUI();

			// Gamma correction shader
			shader = std::unique_ptr<nanogui::GLShader>(new nanogui::GLShader()); 
			shader->init(
				"Render Image Shader",

				"#version 330\n"
				"in vec2 position;\n"
				"out vec2 uv;\n"
				"void main() {\n"
				"    gl_Position = vec4(position.x*2-1, position.y*2-1, 0.0, 1.0);\n"
				"    uv = vec2(position.x, 1-position.y);\n"
				"}",

				"#version 330\n"
				"uniform sampler2D source;\n"
				"in vec2 uv;\n"
				"out vec4 out_color;\n"
				"float toSRGB(float value) {\n"
				"    if (value < 0.0031308)\n"
				"        return 12.92 * value;\n"
				"    return 1.055 * pow(value, 0.41666) - 0.055;\n"
				"}\n"
				"void main() {\n"
				"    vec4 color = texture(source, uv);\n"
				"    out_color = vec4(toSRGB(color.r), toSRGB(color.g), toSRGB(color.b), 1);\n"
				"}"
			);

			// Create two triangles
			Eigen::MatrixXi indices(3, 2);
			indices.col(0) << 0, 1, 2;
			indices.col(1) << 2, 3, 0;

			Eigen::MatrixXf positions(2, 4);
			positions.col(0) << 0, 0;
			positions.col(1) << 1, 0;
			positions.col(2) << 1, 1;
			positions.col(3) << 0, 1;

			shader->bind();
			shader->uploadIndices(indices);
			shader->uploadAttrib("position", positions);

			// Set up texture for drawing on screen
			glGenTextures(1, &texture);
			glBindTexture(GL_TEXTURE_2D, texture);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

			drawAll();
			setVisible(true);
		}

		virtual ~Application() {
			glDeleteTextures(1, &texture);
		}

		void changeIntegrator() {
			renderer->changeIntegrator((Integrator::Type) integratorBox->selectedIndex());
			renderer->refresh();
		}

		void initializeGUI() {
			glfwSetWindowSize(glfwWindow(), renderer->getResolution().x, renderer->getResolution().y);
			window = new nanogui::Window(this, "Settings");
			window->setPosition(nanogui::Vector2i(15, 15));
			window->setLayout(new nanogui::GroupLayout());
			// File open button, 0x0000F1A9 is the code for the folder icon
			nanogui::Button* openButton = new nanogui::Button(window, "Open File", 0x0000F1A9);
			openButton->setCallback([this]() {
				std::pair<std::string, std::string> jsontype = std::pair<std::string, std::string>("json", "Scene File");
				std::vector<std::pair<std::string, std::string> > filetypes;
				filetypes.push_back(jsontype);
				std::string filename = nanogui::file_dialog(filetypes, false);
				renderer->openScene(filename);
				glfwSetWindowSize(glfwWindow(), renderer->getResolution().x, renderer->getResolution().y);
			});

			nanogui::Button* saveButton = new nanogui::Button(window, "Save Render", 0x0000F239);
			saveButton->setCallback([this]() {
				std::pair<std::string, std::string> pngtype = std::pair<std::string, std::string>("png", "Image");
				std::vector<std::pair<std::string, std::string> > filetypes;
				filetypes.push_back(pngtype);
				std::string filename = nanogui::file_dialog(filetypes, true);
				renderer->getImage().save(filename);
			});

			// Integrator box
			new nanogui::Label(window, "Integrator", "sans-bold");
			integratorBox = new nanogui::ComboBox(window, { "PT", "BSDF", "AO" });
			integratorBox->setCallback([&](int) { changeIntegrator(); });
			
			performLayout(mNVGContext);
		}

		void drawContents() {
			// Drawing function for the image. 
			if (renderer->isSceneOpen()) renderer->renderFrame();
			const glm::ivec2& resolution = renderer->getResolution();

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture);
			glPixelStorei(GL_UNPACK_ROW_LENGTH, resolution.x);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, resolution.x, resolution.y, 0, GL_RGB, GL_FLOAT, renderer->getImage().getData());
			glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

			glViewport(0, 0, mPixelRatio * resolution.x, mPixelRatio * resolution.y);
			shader->bind();
			shader->setUniform("source", 0);
			shader->drawIndexed(GL_TRIANGLES, 0, 2);
			glViewport(0, 0, mFBSize[0], mFBSize[1]);
		}
	};
}