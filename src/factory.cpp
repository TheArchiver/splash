#include "./factory.h"

#include <json/json.h>

#include "./camera.h"
#if HAVE_GPHOTO
#include "./colorcalibrator.h"
#endif
#include "./controller_blender.h"
#include "./filter.h"
#include "./geometry.h"
#include "./image.h"
#if HAVE_LINUX
#include "./image_v4l2.h"
#endif
#if HAVE_GPHOTO
#include "./image_gphoto.h"
#endif
#include "./image_ffmpeg.h"
#if HAVE_OPENCV
#include "./image_opencv.h"
#endif
#if HAVE_SHMDATA
#include "./image_shmdata.h"
#endif
#include "./link.h"
#include "./log.h"
#include "./mesh.h"
#if HAVE_SHMDATA
#include "./mesh_shmdata.h"
#include "./sink_shmdata.h"
#include "./sink_shmdata_encoded.h"
#endif
#include "./object.h"
#if HAVE_PYTHON
#include "./controller_pythonEmbedded.h"
#endif
#include "./queue.h"
#include "./scene.h"
#include "./texture.h"
#include "./texture_image.h"
#if HAVE_OSX
#include "./texture_syphon.h"
#endif
#include "./threadpool.h"
#include "./timer.h"
#include "./warp.h"
#include "./window.h"

using namespace std;

namespace Splash
{

/*************/
Factory::Factory()
{
    registerObjects();
}

/*************/
Factory::Factory(weak_ptr<RootObject> root)
{
    _root = root;
    if (!root.expired() && root.lock()->getType() == "scene")
    {
        _isScene = true;
        if (dynamic_pointer_cast<Scene>(root.lock())->isMaster())
            _isMasterScene = true;
    }

    loadDefaults();
    registerObjects();
}

/*************/
void Factory::loadDefaults()
{
    auto defaultEnv = getenv(SPLASH_DEFAULTS_FILE_ENV);
    if (!defaultEnv)
        return;

    auto filename = string(defaultEnv);
    ifstream in(filename, ios::in | ios::binary);
    string contents;
    if (in)
    {
        in.seekg(0, ios::end);
        contents.resize(in.tellg());
        in.seekg(0, ios::beg);
        in.read(&contents[0], contents.size());
        in.close();
    }
    else
    {
        Log::get() << Log::WARNING << "Factory::" << __FUNCTION__ << " - Unable to open file " << filename << Log::endl;
        return;
    }

    Json::Value config;
    Json::Reader reader;

    bool success = reader.parse(contents, config);
    if (!success)
    {
        Log::get() << Log::WARNING << "Factory::" << __FUNCTION__ << " - Unable to parse file " << filename << Log::endl;
        Log::get() << Log::WARNING << reader.getFormattedErrorMessages() << Log::endl;
        return;
    }

    auto objNames = config.getMemberNames();
    for (auto& name : objNames)
    {
        unordered_map<string, Values> defaults;
        auto attrNames = config[name].getMemberNames();
        for (const auto& attrName : attrNames)
        {
            auto value = jsonToValues(config[name][attrName]);
            defaults[attrName] = value;
        }
        _defaults[name] = defaults;
    }
}

/*************/
Values Factory::jsonToValues(const Json::Value& values)
{
    Values outValues;

    if (values.isInt())
        outValues.emplace_back(values.asInt());
    else if (values.isDouble())
        outValues.emplace_back(values.asFloat());
    else if (values.isArray())
        for (const auto& v : values)
        {
            if (v.isInt())
                outValues.emplace_back(v.asInt());
            else if (v.isDouble())
                outValues.emplace_back(v.asFloat());
            else if (v.isArray())
                outValues.emplace_back(jsonToValues(v));
            else
                outValues.emplace_back(v.asString());
        }
    else
        outValues.emplace_back(values.asString());

    return outValues;
}

/*************/
shared_ptr<BaseObject> Factory::create(const string& type)
{
    // Not all object types are listed here, only those which are available to the user are
    auto page = _objectBook.find(type);
    if (page != _objectBook.end())
    {
        auto object = page->second.builder();

        auto defaultIt = _defaults.find(type);
        if (defaultIt != _defaults.end())
            for (auto& attr : defaultIt->second)
                object->setAttribute(attr.first, attr.second);

        if (object)
            object->setCategory(page->second.objectCategory);
        return object;
    }
    else
    {
        Log::get() << Log::WARNING << "Factory::" << __FUNCTION__ << " - Object type " << type << " does not exist" << Log::endl;
        return {nullptr};
    }
}

/*************/
vector<string> Factory::getObjectTypes()
{
    vector<string> types;
    for (auto& page : _objectBook)
        types.push_back(page.first);
    return types;
}

/*************/
vector<string> Factory::getObjectsOfCategory(BaseObject::Category c)
{
    vector<string> types;
    for (auto& page : _objectBook)
        if (page.second.objectCategory == c)
            types.push_back(page.first);

    return types;
}

/*************/
string Factory::getShortDescription(const string& type)
{
    if (!isCreatable(type))
        return "";

    return _objectBook[type].shortDescription;
}

/*************/
string Factory::getDescription(const string& type)
{
    if (!isCreatable(type))
        return "";

    return _objectBook[type].description;
}

/*************/
bool Factory::isCreatable(string type)
{
    auto it = _objectBook.find(type);
    if (it == _objectBook.end())
    {
        Log::get() << Log::WARNING << "Factory::" << __FUNCTION__ << " - Object type " << type << " does not exist" << Log::endl;
        return false;
    }

    return true;
}

/*************/
void Factory::registerObjects()
{
    _objectBook["blender"] = Page([&]() { return dynamic_pointer_cast<BaseObject>(make_shared<Blender>(_root)); }, BaseObject::Category::MISC, "blender");
    _objectBook["camera"] = Page([&]() { return dynamic_pointer_cast<BaseObject>(make_shared<Camera>(_root)); }, BaseObject::Category::MISC, "camera");
    _objectBook["filter"] = Page([&]() { return dynamic_pointer_cast<BaseObject>(make_shared<Filter>(_root)); }, BaseObject::Category::MISC, "filter");
    _objectBook["geometry"] = Page([&]() { return dynamic_pointer_cast<BaseObject>(make_shared<Geometry>(_root)); }, BaseObject::Category::MISC, "Geometry");
    _objectBook["image"] = Page([&]() { return dynamic_pointer_cast<BaseObject>(make_shared<Image>(_root)); }, BaseObject::Category::IMAGE, "image");

#if HAVE_LINUX
    _objectBook["image_v4l2"] = Page(
        [&]() {
            shared_ptr<BaseObject> object;
            if (!_isScene)
                object = dynamic_pointer_cast<BaseObject>(make_shared<Image_V4L2>(_root));
            else
                object = dynamic_pointer_cast<BaseObject>(make_shared<Image>(_root));
            return object;
        },
        BaseObject::Category::IMAGE,
        "Video4Linux2 input device");
#endif

    _objectBook["image_ffmpeg"] = Page(
        [&]() {
            shared_ptr<BaseObject> object;
            if (!_isScene)
                object = dynamic_pointer_cast<BaseObject>(make_shared<Image_FFmpeg>(_root));
            else
                object = dynamic_pointer_cast<BaseObject>(make_shared<Image>(_root));
            return object;
        },
        BaseObject::Category::IMAGE,
        "video");

#if HAVE_GPHOTO
    _objectBook["image_gphoto"] = Page(
        [&]() {
            shared_ptr<BaseObject> object;
            if (!_isScene)
                object = dynamic_pointer_cast<BaseObject>(make_shared<Image_GPhoto>(_root));
            else
                object = dynamic_pointer_cast<BaseObject>(make_shared<Image>(_root));
            return object;
        },
        BaseObject::Category::IMAGE,
        "digital camera");
#endif

#if HAVE_SHMDATA
    _objectBook["image_shmdata"] = Page(
        [&]() {
            shared_ptr<BaseObject> object;
            if (!_isScene)
                object = dynamic_pointer_cast<BaseObject>(make_shared<Image_Shmdata>(_root));
            else
                object = dynamic_pointer_cast<BaseObject>(make_shared<Image>(_root));
            return object;
        },
        BaseObject::Category::IMAGE,
        "video through shared memory");
#endif

#if HAVE_OPENCV
    _objectBook["image_opencv"] = Page(
        [&]() {
            shared_ptr<BaseObject> object;
            if (!_isScene)
                object = dynamic_pointer_cast<BaseObject>(make_shared<Image_OpenCV>(_root));
            else
                object = dynamic_pointer_cast<BaseObject>(make_shared<Image>(_root));
            return object;
        },
        BaseObject::Category::IMAGE,
        "camera through opencv");
#endif

    _objectBook["mesh"] = Page([&]() { return dynamic_pointer_cast<BaseObject>(make_shared<Mesh>(_root)); }, BaseObject::Category::MESH, "mesh from obj file");

#if HAVE_SHMDATA
    _objectBook["mesh_shmdata"] = Page(
        [&]() {
            shared_ptr<BaseObject> object;
            if (!_isScene)
                object = dynamic_pointer_cast<BaseObject>(make_shared<Mesh_Shmdata>(_root));
            else
                object = dynamic_pointer_cast<BaseObject>(make_shared<Mesh>(_root));
            return object;
        },
        BaseObject::Category::MESH,
        "mesh through shared memory");

    _objectBook["sink_shmdata"] =
        Page([&]() { return dynamic_pointer_cast<BaseObject>(make_shared<Sink_Shmdata>(_root)); }, BaseObject::Category::MISC, "sink a texture to shmdata file");

    _objectBook["sink_shmdata_encoded"] = Page([&]() { return dynamic_pointer_cast<BaseObject>(make_shared<Sink_Shmdata_Encoded>(_root)); },
        BaseObject::Category::MISC,
        "sink a texture as an encoded video to shmdata file");
#endif

    _objectBook["object"] = Page([&]() { return dynamic_pointer_cast<BaseObject>(make_shared<Object>(_root)); }, BaseObject::Category::MISC, "object");

#if HAVE_PYTHON
    _objectBook["python"] = Page(
        [&]() {
            if (!_isMasterScene && !_root.expired())
                return shared_ptr<BaseObject>(nullptr);
            return dynamic_pointer_cast<BaseObject>(make_shared<PythonEmbedded>(_root));
        },
        BaseObject::Category::MISC,
        "python");
#endif

    _objectBook["queue"] = Page(
        [&]() {
            shared_ptr<BaseObject> object;
            if (!_isScene)
                object = dynamic_pointer_cast<BaseObject>(make_shared<Queue>(_root));
            else
                object = dynamic_pointer_cast<BaseObject>(make_shared<QueueSurrogate>(_root));
            return object;
        },
        BaseObject::Category::IMAGE,
        "video queue");

#if HAVE_OSX
    _objectBook["texture_syphon"] =
        Page([&]() { return dynamic_pointer_cast<BaseObject>(make_shared<Texture_Syphon>(_root)); }, BaseObject::Category::IMAGE, "texture image through Syphon");
#endif

    _objectBook["warp"] = Page([&]() { return dynamic_pointer_cast<BaseObject>(make_shared<Warp>(_root)); }, BaseObject::Category::MISC, "warp");
    _objectBook["window"] = Page([&]() { return dynamic_pointer_cast<BaseObject>(make_shared<Window>(_root)); }, BaseObject::Category::MISC, "window");
}

} // end of namespace
