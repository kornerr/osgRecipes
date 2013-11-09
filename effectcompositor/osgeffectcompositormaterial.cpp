#include <osg/io_utils>
#include <osg/MatrixTransform>
#include <osg/PolygonMode>
#include <osgDB/ReadFile>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

#include "EffectCompositor"

osg::Geode *createScreenQuad(float width,
                             float height,
                             float scale = 1,
                             osg::Vec3 corner = osg::Vec3())
{
    osg::Geometry* geom = osg::createTexturedQuadGeometry(
        corner,
        osg::Vec3(width, 0, 0),
        osg::Vec3(0, height, 0),
        0,
        0,
        scale,
        scale);
    osg::ref_ptr<osg::Geode> quad = new osg::Geode;
    quad->addDrawable(geom);
    int values = osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED;
    quad->getOrCreateStateSet()->setAttribute(
        new osg::PolygonMode(osg::PolygonMode::FRONT_AND_BACK,
                             osg::PolygonMode::FILL),
        values);
    quad->getOrCreateStateSet()->setMode(GL_LIGHTING, values);
    return quad.release();
}

osg::Camera *createHUDCamera(double left   = 0,
                             double right  = 1,
                             double bottom = 0,
                             double top    = 1)
{
    osg::ref_ptr<osg::Camera> camera = new osg::Camera;
    camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    camera->setClearMask(GL_DEPTH_BUFFER_BIT);
    camera->setRenderOrder(osg::Camera::POST_RENDER);
    camera->setAllowEventFocus(false);
    camera->setProjectionMatrix(osg::Matrix::ortho2D(left, right, bottom, top));
    camera->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
    return camera.release();
}

osg::ref_ptr<osg::Camera> createTextureDisplayQuad(const osg::Vec3 &pos,
                                                   osg::StateAttribute *tex,
                                                   float scale,
                                                   float width  = 0.3,
                                                   float height = 0.2)
{
    osg::ref_ptr<osg::Camera> hc = createHUDCamera();
    hc->addChild(createScreenQuad(width, height, scale, pos));
    hc->getOrCreateStateSet()->setTextureAttributeAndModes(0, tex);
    return hc;
}

int main( int argc, char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );
    osgViewer::Viewer viewer;
    
    std::string effectFile("pipeline.xml");
    arguments.read( "--effect", effectFile );
    
    // Create the scene
    osg::Node* model = osgDB::readNodeFiles( arguments );
    if ( !model ) model = osgDB::readNodeFile( "box.osgt" );
    osg::Vec3 lightPos(-1, -4, 4);
    osg::LightSource* light = new osg::LightSource;
    light->getLight()->setPosition(osg::Vec4(lightPos.x(), lightPos.y(), lightPos.z(), 1));
    osg::MatrixTransform* box1 = new osg::MatrixTransform;
    box1->addChild(model);
    box1->setMatrix(osg::Matrix::translate(-3, 0, 0));
    osg::MatrixTransform* box2 = new osg::MatrixTransform;
    box2->addChild(model);
    box2->setMatrix(osg::Matrix::translate(3, 0, 0));
    
    osg::ref_ptr<osg::Group> scene = new osg::Group;
    scene->addChild( light );
    scene->addChild( box1 );
    scene->addChild( box2 );
    
    // Create the effect compositor from XML file
    osgFX::EffectCompositor* compositor = osgFX::readEffectFile( effectFile );
    if ( !compositor )
    {
        OSG_WARN << "Effect file " << effectFile << " can't be loaded!" << std::endl;
        return 1;
    }
    // Light position for the 2nd pass.
    osgFX::EffectCompositor::PassData pass2;
    compositor->getPassData("pass2", pass2);
    pass2.pass->getOrCreateStateSet()->addUniform(new osg::Uniform("lightPos", lightPos));
    // For the fastest and simplest effect use, this is enough!
    compositor->addChild( scene.get() );
    int textureSize = compositor->getTexture("pass2Final")->getTextureWidth();
    
    // Final texture display.
    osg::ref_ptr<osg::Camera> qTexFinal =
        createTextureDisplayQuad(osg::Vec3(0, 0, 0),
                                 compositor->getTexture("pass2Final"),
                                 textureSize,
                                 1,
                                 1);
    // Add all to the root node of the viewer
    osg::ref_ptr<osg::Group> root = new osg::Group;
    root->addChild( compositor );
    root->addChild(qTexFinal);
    viewer.addEventHandler( new osgViewer::WindowSizeHandler );
    viewer.setSceneData( root.get() );
    
    viewer.setUpViewOnSingleScreen( 0 );
    return viewer.run();
}
