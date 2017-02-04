/*
 * Copyright (C) 2015 Emmanuel Durand
 *
 * This file is part of Splash.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Splash is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Splash.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * @filter.h
 * The Filter class
 */

#ifndef SPLASH_FILTER_H
#define SPLASH_FILTER_H

#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>

#include "config.h"

#include "basetypes.h"
#include "coretypes.h"
#include "image.h"
#include "object.h"
#include "texture.h"
#include "texture_image.h"

namespace Splash
{

/*************/
class Filter : public Texture
{
  public:
    /**
     * \brief Constructor
     * \param root Root object
     */
    Filter(std::weak_ptr<RootObject> root);

    /**
     * \brief Destructor
     */
    ~Filter();

    /**
     * No copy constructor, but a move one
     */
    Filter(const Filter&) = delete;
    Filter(Filter&&) = default;
    Filter& operator=(const Filter&) = delete;

    /**
     * \brief Bind the filter
     */
    void bind();

    /**
     * \brief Unbind the filter
     */
    void unbind();

    /**
     * Get the output texture
     * \return Return the output texture
     */
    std::shared_ptr<Texture> getOutTexture() const { return _outTexture; }

    /**
     * Get the shader parameters related to this texture
     * Texture should be locked first
     */
    std::unordered_map<std::string, Values> getShaderUniforms() const;

    /**
     * \brief Get specs of the texture
     * \return Return the texture specs
     */
    ImageBufferSpec getSpec() const { return _outTextureSpec; }

    /**
     * \brief Try to link the given BaseObject to this object
     * \param obj Shared pointer to the (wannabe) child object
     */
    bool linkTo(std::shared_ptr<BaseObject> obj);

    /**
     * \brief Try to unlink the given BaseObject from this object
     * \param obj Shared pointer to the (supposed) child object
     */
    void unlinkFrom(std::shared_ptr<BaseObject> obj);

    /**
     * \brief Filters should always be saved as it holds user-modifiable parameters
     * \param savable Needed for heritage reasons, no effect whatsoever
     */
    void setSavable(bool savable) { _savable = true; }

    /**
     * \brief Render the filter
     */
    void render();

    /**
     * \brief Update for a filter does nothing, it is the render() job
     */
    void update() {}

  private:
    bool _isInitialized{false};
    std::shared_ptr<GlWindow> _window;
    std::vector<std::weak_ptr<Texture>> _inTextures;

    GLuint _fbo{0};
    std::shared_ptr<Texture_Image> _outTexture{nullptr};
    std::shared_ptr<Object> _screen;
    ImageBufferSpec _outTextureSpec;

    // Filter parameters
    std::unordered_map<std::string, Values> _filterUniforms; //!< Contains all filter uniforms
    bool _render16bits{false};                               //!< Set to true for the filter to be rendered in 16bits
    bool _updateColorDepth{false};                           //!< Set to true if the _render16bits has been updated

    std::string _shaderSource{""};     //!< User defined fragment shader filter
    std::string _shaderSourceFile{""}; //!< User defined fragment shader filter source file

    // Tasks queue
    std::mutex _taskMutex;
    std::list<std::function<void()>> _taskQueue;

    /**
     * \brief Add a new task to the queue
     * \param task Task function
     */
    void addTask(const std::function<void()>& task)
    {
        std::lock_guard<std::mutex> lock(_taskMutex);
        _taskQueue.push_back(task);
    }

    /**
     * \brief Init function called in constructors
     */
    void init();

    /**
     * \brief Set the filter fragment shader. Automatically adds attributes corresponding to the uniforms
     * \param source Source fragment shader
     * \return Return true if the shader is valid
     */
    bool setFilterSource(const std::string& source);

    /**
     * \brief Setup the output texture
     */
    void setOutput();

    /**
     * \brief Update the color depth for all textures
     */
    void updateColorDepth();

    /**
     * \brief Updates the shader uniforms according to the textures and images the filter is connected to.
     */
    void updateUniforms();

    /**
     * \brief Register new functors to modify attributes
     */
    void registerAttributes();
};

} // end of namespace

#endif // SPLASH_FILTER_H
