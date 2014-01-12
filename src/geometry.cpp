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
}

/*************/
void Geometry::activate() const
{
    glBindVertexArray(_vertexArray);
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
        glDeleteVertexArrays(1, &_vertexArray);
        glDeleteBuffers(1, &_vertexCoords);
        glDeleteBuffers(1, &_texCoords);

        glGenVertexArrays(1, &_vertexArray);
        glBindVertexArray(_vertexArray);

        glGenBuffers(1, &_vertexCoords);
        glBindBuffer(GL_ARRAY_BUFFER, _vertexCoords);
        vector<float> vertices = _mesh->getVertCoords();
        _verticesNumber = vertices.size() / 4;
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(_vertexCoords, 4, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(_vertexCoords);
        
        glGenBuffers(1, &_texCoords);
        glBindBuffer(GL_ARRAY_BUFFER, _texCoords);
        vector<float> texcoords = _mesh->getUVCoords();
        glBufferData(GL_ARRAY_BUFFER, texcoords.size() * sizeof(float), texcoords.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(_texCoords, 2, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(_texCoords);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        // TODO: there is a bug with this and new cameras
        //_timestamp = _mesh->getTimestamp();
    }
}


/*************/
void Geometry::registerAttributes()
{
}

} // end of namespace
