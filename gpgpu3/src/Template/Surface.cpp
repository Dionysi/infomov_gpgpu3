#include "stdfax.h"
#include "Surface.h"


Surface::Surface(unsigned int width, unsigned int height)
	: m_Width(width), m_Height(height) {

	m_Pixels = (Color*)malloc(sizeof(Color) * width * height);

	// Create our render texture.
	glGenTextures(1, &m_RenderTexture);
	glBindTexture(GL_TEXTURE_2D, m_RenderTexture);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

	glBindTexture(GL_TEXTURE_2D, 0);
	// Initialize our shader.
	m_Shader = new GLshader("simple_tex.vert", "simple_tex.frag");
	m_VertexBuffer = new GLbuffer(GL_ARRAY_BUFFER, sizeof(c_RenderQuad));
	m_VertexBuffer->Write(sizeof(c_RenderQuad), c_RenderQuad, GL_STATIC_DRAW);

	m_UVbuffer = new GLbuffer(GL_ARRAY_BUFFER, sizeof(c_UV));
	m_UVbuffer->Write(sizeof(c_UV), c_UV, GL_STATIC_DRAW);

	m_IndexBuffer = new GLbuffer(GL_ELEMENT_ARRAY_BUFFER, sizeof(c_Indices));
	m_IndexBuffer->Write(sizeof(c_Indices), c_Indices, GL_STATIC_DRAW);

	m_Shader->Activate();
	m_Shader->SetBufferFloat3(m_VertexBuffer, 0, 0);
	m_Shader->SetBufferFloat2(m_UVbuffer, 1, 0);
	m_Shader->Deactivate();

}

Surface::~Surface() {
	delete m_VertexBuffer;
	delete m_UVbuffer;
	delete m_IndexBuffer;
	delete m_Shader;

	free(m_Pixels);
}

void Surface::Draw() {
	m_Shader->Activate();
	glBindTexture(GL_TEXTURE_2D, m_RenderTexture);
	m_Shader->DrawTriangles(6, m_IndexBuffer, GL_UNSIGNED_INT);
	m_Shader->Deactivate();
}

void Surface::SyncPixels()
{
	glBindTexture(GL_TEXTURE_2D, m_RenderTexture);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_Width, m_Height, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)m_Pixels);
	glBindTexture(GL_TEXTURE_2D, 0);
	glFinish();
}

void Surface::SyncPixels(uint dx, uint dy, uint width, uint height, Color* pixels)
{
	glBindTexture(GL_TEXTURE_2D, m_RenderTexture);
	glTexSubImage2D(GL_TEXTURE_2D, 0, dx, dy, width, height, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)pixels);
	glBindTexture(GL_TEXTURE_2D, 0);
	glFinish();
}

void Surface::PlotPixel(Color color, uint x, uint y)
{
	m_Pixels[x + y * m_Width] = color;
}

void Surface::PlotPixels(Color* colors)
{
	for (int y = 0; y < m_Height; y++) memcpy(m_Pixels, &colors[y * m_Width], sizeof(Color) * m_Width);
}

void Surface::PlotPixels(Color* colors, uint dx, uint dy, uint width, uint height) {
	// Check that we do never write outside of the pixel buffer.
	assert(dx + width < m_Width);
	assert(dy + height < m_Height);

	// Loop over each row of pixels.
	for (int y = 0; y < height; y++)
	{
		// Compute the screen pixel index.
		memcpy(&m_Pixels[dx + (dy + y) * m_Width], &colors[y * width], sizeof(Color) * width);
	}
}

void Surface::Clear()
{
	// Loop over each row of pixels.
	for (int y = 0; y < m_Height; y++)
	{
		// Compute the screen pixel index.
		memset(&m_Pixels[y * m_Width], 0, sizeof(Color) * m_Width);
	}
}
