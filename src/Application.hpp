#pragma once
#include <nanogui/nanogui.h>
#include <nanogui/glutil.h>
#include <nanogui/screen.h>
#include <nanogui/layout.h>

class Application : public nanogui::Screen {
private:
	uint32_t m_texture = 0;
	nanogui::GLShader* m_shader = nullptr;

public:
	Application() : nanogui::Screen(Eigen::Vector2i(1024, 768), "lykta") {
		initialize_gui();

		// Gamma correction shader
		m_shader = new nanogui::GLShader();
		m_shader->init(
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

		m_shader->bind();
		m_shader->uploadIndices(indices);
		m_shader->uploadAttrib("position", positions);

		// Set up texture for drawing on screen
		glGenTextures(1, &m_texture);
		glBindTexture(GL_TEXTURE_2D, m_texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		
		drawAll();
		setVisible(true);
	}

	virtual ~Application() {
		glDeleteTextures(1, &m_texture);
		delete m_shader;
	}

	void initialize_gui() {
		// TODO: Add windows and so on
		glfwSetWindowSize(glfwWindow(), 1024, 768);
		performLayout(mNVGContext);
	}

	void drawContents() {
		// Drawing function for the image. 
		// TODO: Replace temp image with an actual buffer of data
		std::vector<float> temp_image(1024 * 768 * 3, 1.0f);

		// TODO: Allow change of screen size based on render resolution
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_texture);
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 768);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 1024, 768, 0, GL_RGB, GL_FLOAT, temp_image.data());
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

		glViewport(0, 0, mPixelRatio * 1024, mPixelRatio * 768);
		m_shader->bind();
		m_shader->setUniform("source", 0);
		m_shader->drawIndexed(GL_TRIANGLES, 0, 2);
		glViewport(0, 0, mFBSize[0], mFBSize[1]);
	}
};
