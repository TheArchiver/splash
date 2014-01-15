#include "geometry.h"

using namespace std;

namespace Splash {

/*************/
Geometry::Geometry()
{
    _type = "geometry";

    _mesh.reset(new Mesh());
    update();
    _timestamp = _mesh->getTimestamp();
}

/*************/
Geometry::~Geometry()
{
    glDeleteBuffers(1, &_vertexCoords);
    glDeleteBuffers(1, &_texCoords);
    glDeleteBuffers(1, &_normals);
    for (auto v : _vertexArray)
        glDeleteVertexArrays(1, &(v.second));
}

/*************/
void Geometry::activate()
{
    glBindVertexArray(_vertexArray[glfwGetCurrentContext()]);
}

/*************/
void Geometry::deactivate() const
{
    glBindVertexArray(0);
}

/*************/
void Geometry::update()
{
    if (!_mesh)
        return; // No mesh, no update

    // Update the vertex buffers if mesh was updated
    if (_timestamp != _mesh->getTimestamp())
    {
        glDeleteBuffers(1, &_vertexCoords);
        glDeleteBuffers(1, &_texCoords);
        glDeleteBuffers(1, &_normals);

        glGenBuffers(1, &_vertexCoords);
        glBindBuffer(GL_ARRAY_BUFFER, _vertexCoords);
        vector<float> vertices = _mesh->getVertCoords();
        _verticesNumber = vertices.size() / 4;
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
        
        glGenBuffers(1, &_texCoords);
        glBindBuffer(GL_ARRAY_BUFFER, _texCoords);
        vector<float> texcoords = _mesh->getUVCoords();
        glBufferData(GL_ARRAY_BUFFER, texcoords.size() * sizeof(float), texcoords.data(), GL_STATIC_DRAW);

        glGenBuffers(1, &_normals);
        glBindBuffer(GL_ARRAY_BUFFER, _normals);
        vector<float> normals = _mesh->getNormals();
        glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float), normals.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        for (auto v : _vertexArray)
            glDeleteVertexArrays(1, &(v.second));
        _vertexArray.clear();

        _timestamp = _mesh->getTimestamp();
    }

    GLFWwindow* context = glfwGetCurrentContext();
    if (_vertexArray.find(context) == _vertexArray.end())
    {
        _vertexArray[context] = 0;
        
        glGenVertexArrays(1, &(_vertexArray[context]));
        glBindVertexArray(_vertexArray[context]);

        glBindBuffer(GL_ARRAY_BUFFER, _vertexCoords);
        glVertexAttribPointer(_vertexCoords, 4, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(_vertexCoords);

        glBindBuffer(GL_ARRAY_BUFFER, _texCoords);
        glVertexAttribPointer(_texCoords, 2, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(_texCoords);

        glBindBuffer(GL_ARRAY_BUFFER, _normals);
        glVertexAttribPointer(_normals, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(_normals);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
}

/*************/
void Geometry::registerAttributes()
{
}

} // end of namespace
