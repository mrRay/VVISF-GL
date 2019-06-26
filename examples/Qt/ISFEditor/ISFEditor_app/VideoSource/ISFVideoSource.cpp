#include "ISFVideoSource.h"

#include <QFile>

#include "AudioController.h"
#include "ISFController.h"




using namespace VVGL;
using namespace VVISF;




ISFVideoSource::ISFVideoSource(QObject *parent)	{
	Q_UNUSED(parent);
	//scene = nullptr;
	
	ISFController	*isfc = GetISFController();
	QThread			*renderThread = isfc->renderThread();
	GLBufferPoolRef			isfPool = isfc->renderThreadBufferPool();
	GLTexToTexCopierRef		isfCopier = isfc->renderThreadTexCopier();
	
	scene = CreateISFSceneRef();
	scene->context()->moveToThread(renderThread);
	scene->setPrivatePool(isfPool);
	scene->setPrivateCopier(isfCopier);
}
ISFVideoSource::~ISFVideoSource()	{
	scene = nullptr;
}


//VVGL::GLBufferRef getBuffer() override;
QList<MediaFile> ISFVideoSource::createListOfStaticMediaFiles()	{
	QList<MediaFile>		returnMe;
	
	const char		*tmpNames[] = {
		"Alien_Cavity.fs",
		"Auroras.fs",
		"Bechamel.fs",
		"BlackCherryCosmos.fs",
		"Bubbles.fs",
		"Candy_core.fs",
		"Cellular.fs",
		"Chanel_Doodle_1.fs",
		"Chanel_Doodle_2.fs",
		"Chanel_Final.fs",
		"Color_palette.fs",
		"Demo-Volumetric_Lines.fs",
		"Doodling_3-Glass.fs",
		"Doodling_8-Penumbra.fs",
		"Electric_Field.fs",
		"Flower_Orbit_Traps.fs",
		"Fractal_Land.fs",
		"Geometric_Descent.fs",
		"Goto10.fs",
		"h_e_e_e_l_i_x.fs",
		"hhhzzzsss_Orbit_Traps.fs",
		"HyperbolicSpace.fs",
		"HyperbolicSpace2.fs",
		"hypnosis_rgb_audio.fs",
		"Mandala_elevator.fs",
		"ngWaves11.fs",
		"perlin_noise_in_polar_coordinate.fs",
		"Plasma_Globe.fs",
		"Rainbow_Worm_0.fs",
		"Random_Notes.fs",
		"RingsOfFire.fs",
		"Shaderdough_fairy.fs",
		"solar-vixzion.fs",
		"SoundEclipse.fs",
		"Spiral_2_(Yellow_Pink).fs",
		"Spiral_of_Spirals_2.fs",
		"Star_Nest.fs",
		"Test Card.fs",
		"The_Drive_Home.fs",
		"Tunnel_#1.fs",
		"Voxel_Edges.fs"
	};
	for (unsigned long i=0; i<sizeof(tmpNames)/sizeof(const char *); ++i)	{
		const char		*tmpName = tmpNames[i];
		QString			fullPath = QString(":/src_ISFs/%1").arg(QString(tmpName));
		returnMe.append(MediaFile(MediaFile::Type_ISF, fullPath));
	}
	
	return returnMe;
}
void ISFVideoSource::start()	{
	//qDebug() << __PRETTY_FUNCTION__;
	
	std::lock_guard<std::recursive_mutex> tmpLock(_lock);
	if (_running)
		return;
	
	_running=true;
	/*
	if (scene == nullptr)	{
		scene = CreateISFSceneRef();
	}
	
	GLContextRef		sceneCtx = (scene==nullptr) ? nullptr : scene->context();
	ISFController		*isfc = GetISFController();
	VVGLRenderQThread	*isfRenderThread = (isfc==nullptr) ? nullptr : isfc->renderThread();
	
	GLBufferPoolRef			isfPool = isfc->renderThreadBufferPool();
	scene->setPrivatePool(isfPool);
	GLTexToTexCopierRef		isfCopier = isfc->renderThreadTexCopier();
	scene->setPrivateCopier(isfCopier);
	
	if (sceneCtx!=nullptr && isfRenderThread!=nullptr)	{
		sceneCtx->moveToThread(isfRenderThread);
//		qDebug() << "just moved Dynamic ISFVideoSource to render thread...";
	}
//	else	{
//		qDebug() << "ERR: scene or isf render thread NULL, " << __PRETTY_FUNCTION__;
//	}
	*/
	
	if (_file.type()==MediaFile::Type_ISF)	{
		
		QFile			tmpFile(_file.path());
		if (tmpFile.open(QFile::ReadOnly))	{
			QTextStream		rStream(&tmpFile);
			QString			rawSrc = rStream.readAll();
			std::string		pathToDir(":/src_ISFs");
			ISFDocRef		tmpDoc = CreateISFDocRefWith(rawSrc.toStdString(), pathToDir, ISFVertPassthru_GL2, &(*scene), false);
			scene->useDoc(tmpDoc);
			
			tmpFile.close();
		}
		else
			qDebug() << "ERR: couldnt open ISF src " << _file.path();
		
	}
}
void ISFVideoSource::stop()	{
	//qDebug() << __PRETTY_FUNCTION__;
	
	std::lock_guard<std::recursive_mutex> tmpLock(_lock);
	
	VideoSource::stop();
	/*
	scene = nullptr;
	*/
}
bool ISFVideoSource::playingBackItem(const MediaFile & n)	{
	if (n.type()==MediaFile::Type_ISF		&&
	_file.type()==MediaFile::Type_ISF		&&
	_file.path()==n.path())	{
		return true;
	}
	return false;
}
void ISFVideoSource::loadFile(const MediaFile & n)	{
	//qDebug() << __PRETTY_FUNCTION__ << ", " << n;
	
	if (n.type() != MediaFile::Type_ISF)
		return;
	
	std::lock_guard<std::recursive_mutex> tmpLock(_lock);
	if (_file == n)
		return;
	
	stop();
	_file = n;
	start();
}




void ISFVideoSource::renderABuffer()	{
	if (_file.type() != MediaFile::Type_ISF)
		return;
	
	AudioController		*ac = GetAudioController();
	
	std::lock_guard<std::recursive_mutex> tmpLock(_lock);
	
	//	the scene needs a private pool & copier (it's being rendered on the dedicated rendering thread)
	if (scene->privatePool() == nullptr)	{
		ISFController			*isfc = GetISFController();
		GLBufferPoolRef			isfPool = isfc->renderThreadBufferPool();
		GLTexToTexCopierRef		isfCopier = isfc->renderThreadTexCopier();
		if (isfPool == nullptr || isfCopier == nullptr)
			return;
		scene->setPrivatePool(isfPool);
		scene->setPrivateCopier(isfCopier);
	}
	
	ISFDocRef		sceneDoc = scene->doc();
	if (sceneDoc == nullptr)
		return;
	GLBufferRef		audioBuffer = (ac==nullptr) ? nullptr : ac->getAudioImageBuffer();
	GLBufferRef		audioFFTBuffer = (ac==nullptr) ? nullptr : ac->getAudioFFTBuffer();
	
	for (const ISFAttrRef & audioInput : sceneDoc->audioInputs())	{
		if (audioInput == nullptr)
			continue;
		
		switch (audioInput->type())	{
		case ISFValType_Audio:
			scene->setValueForInputNamed(ISFImageVal(audioBuffer), audioInput->name());
			break;
		case ISFValType_AudioFFT:
			scene->setValueForInputNamed(ISFImageVal(audioFFTBuffer), audioInput->name());
			break;
		default:	//	intentionally blank, do nothing
			break;
		}
	}
	
	GLBufferRef		newBuffer = scene->createAndRenderABuffer(_size);
	emit frameProduced(newBuffer);
}
