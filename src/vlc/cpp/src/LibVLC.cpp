/*
#include <Windows.h>
#include <stdint.h>
#include <io.h>
#include <stdio.h>
#include <fcntl.h>
*/
#include <mutex>
#include <iostream>
#include <string>
#include <StdInt.h>
//#include "main.h"
#include <windows.h> 

using std::string;
using namespace std;

/////////////////////////////////////////////////////////////////////////////////////

//t_ctx ctx;
//uint8_t* pixel;

LibVLC::LibVLC(void)
{
	char const *Args[] =
	{
		//"--aout", "amem",
		"--drop-late-frames",
		"--ignore-config",
		"--intf", "dummy",
		"--no-disable-screensaver",
		"--no-snapshot-preview",
		"--no-stats",
		"--no-video-title-show",
		"--text-renderer", "dummy",
		"--quiet",
		//"--no-xlib", //no xlib if linux
		//"--vout", "vmem"
		//"--avcodec-hw=dxva2",
		//"--verbose=2"
	};	
	
	int Argc = sizeof(Args) / sizeof(*Args);
	libVlcInstance = libvlc_new(Argc, Args);
	//libVlcInstance = libvlc_new(0, NULL);
	
	//std::cout << "created " << libVlcInstance << std::endl;
}
//#if PLATFORM_LINUX
//"--no-xlib",
//#endif
//#if DEBUG
//"--verbose=2",
//#else
//#endif

LibVLC::~LibVLC(void)
{ 
    libvlc_event_detach( eventManager, libvlc_MediaPlayerSnapshotTaken, 	callbacks, this );
    libvlc_event_detach( eventManager, libvlc_MediaPlayerTimeChanged, 		callbacks, this );
    libvlc_event_detach( eventManager, libvlc_MediaPlayerPlaying, 			callbacks, this );
    libvlc_event_detach( eventManager, libvlc_MediaPlayerPaused, 			callbacks, this );
    libvlc_event_detach( eventManager, libvlc_MediaPlayerStopped, 			callbacks, this );
    libvlc_event_detach( eventManager, libvlc_MediaPlayerEndReached, 		callbacks, this );
    libvlc_event_detach( eventManager, libvlc_MediaPlayerPositionChanged,	callbacks, this );
    //stop();
    libvlc_media_player_release( libVlcMediaPlayer );	
	libvlc_release( libVlcInstance );
	
	delete libVlcInstance;
	delete libVlcMediaItem;
	delete libVlcMediaPlayer;
	
	delete ctx.pixeldata;
}
/*
LibVLC* LibVLC::i=NULL;
LibVLC* LibVLC::create()
{
    if(!i)
	{
        i=new LibVLC;
    }
    return i;
}
*/
LibVLC* LibVLC::create()
{
    return new LibVLC;
}

/////////////////////////////////////////////////////////////////////////////////////

static void *lock(void *data, void **p_pixels)
{
	t_ctx *ctx = (t_ctx*)data;
	ctx->imagemutex.lock();
	if (ctx->bufferFlip)
		*p_pixels = ctx->pixeldata;
	else
		*p_pixels = ctx->pixeldata2;
	ctx->bufferFlip = !(ctx->bufferFlip);
	return NULL;
}

static void unlock(void *data, void *id, void *const *p_pixels)
{
	t_ctx *ctx = (t_ctx *)data;
	ctx->imagemutex.unlock();
}

static void display(void *opaque, void *picture)
{
	//t_ctx *ctx = (t_ctx *)data;
	//self->flags[15]=1;
	//std::cout << "display " << self << std::endl;
}

static unsigned format_setup(void** opaque, char* chroma, unsigned* width, unsigned* height, unsigned* pitches, unsigned* lines)
{
    //LibVLC* self = reinterpret_cast<LibVLC*>( opaque );
	struct ctx *callback = reinterpret_cast<struct ctx *>(*opaque);	
	
	unsigned _w = (*width);
	unsigned _h = (*height);
	unsigned _pitch = _w*4;
	unsigned _frame = _w*_h*4;
	
	(*pitches) = _pitch;
	(*lines) = _h;
	memcpy(chroma, "RV32", 4);
	
	if (callback->pixeldata != 0)
		delete callback->pixeldata;
	if (callback->pixeldata2 != 0)
		delete callback->pixeldata2;
		
	callback->pixeldata = new unsigned char[_frame];
	callback->pixeldata2 = new unsigned char[_frame];
	return 1;
}

static void format_cleanup(void *opaque)
{
}

/////////////////////////////////////////////////////////////////////////////////////

uint8_t* LibVLC::getPixelData()
{
	//return pixels;
	if (ctx.bufferFlip)
		return ctx.pixeldata2;
	else
		return ctx.pixeldata;
}

void LibVLC::setPath(const char* path)
{
	//std::cout << "set location: " << path << std::endl;
	//libVlcMediaItem = libvlc_media_new_path(libVlcInstance, path);
	libVlcMediaItem = libvlc_media_new_location(libVlcInstance, path);
	//libVlcMediaItem = libvlc_media_new_location(libVlcInstance, "file:///C:\\Program Files (x86)\\Xms Client 3\\resources\\downloaded\\files\\ac079337-dbd1-11e6-a59e-f681aa9a2e27.mp4");
	libVlcMediaPlayer = libvlc_media_player_new_from_media(libVlcMediaItem);
	libvlc_media_parse(libVlcMediaItem);
	libvlc_media_release(libVlcMediaItem);
	useHWacceleration(true);
	if (libVlcMediaItem!=nullptr)
	{
		std::string sa = "input-repeat=";
		sa += std::to_string(repeat);
		libvlc_media_add_option(libVlcMediaItem, sa.c_str() );	
		//if (repeat==-1)
			//libvlc_media_add_option(libVlcMediaItem, "input-repeat=-1" );	
		//else if (repeat==0)
			//libvlc_media_add_option(libVlcMediaItem, "input-repeat=0" );	
		//std::cout << "Num repeats: " << sa << std::endl;
	}
}

void LibVLC::play()
{
	libvlc_media_player_play(libVlcMediaPlayer);
}

void LibVLC::play(const char* path)
{
	setPath(path);
	ctx.pixeldata = 0;
	ctx.pixeldata2 = 0;
		
	libvlc_video_set_format_callbacks(libVlcMediaPlayer, format_setup, format_cleanup);
	libvlc_video_set_callbacks(libVlcMediaPlayer, lock, unlock, display, &ctx);
	eventManager = libvlc_media_player_event_manager( libVlcMediaPlayer );
	registerEvents();
	libvlc_media_player_play(libVlcMediaPlayer);
	libvlc_audio_set_volume(libVlcMediaPlayer, 0);
}

void LibVLC::playInWindow()
{
	//libvlc_video_set_format_callbacks(libVlcMediaPlayer, format_setup, format_cleanup);
	ctx.pixeldata = 0;
	ctx.pixeldata2 = 0;
	eventManager = libvlc_media_player_event_manager( libVlcMediaPlayer );
	registerEvents();
	libvlc_media_player_play(libVlcMediaPlayer);
	//libvlc_audio_set_volume(libVlcMediaPlayer, 0);
}

void LibVLC::playInWindow(const char* path)
{
	setPath(path);
	ctx.pixeldata = 0;
	ctx.pixeldata2 = 0;
	//libvlc_video_set_format_callbacks(libVlcMediaPlayer, format_setup, format_cleanup);
	eventManager = libvlc_media_player_event_manager( libVlcMediaPlayer );
	registerEvents();
	libvlc_media_player_play(libVlcMediaPlayer);
	//libvlc_audio_set_volume(libVlcMediaPlayer, 0);
}

void LibVLC::setInitProps()
{
	setVolume(vol);
}

void LibVLC::stop()
{
	libvlc_media_player_stop(libVlcMediaPlayer);
}

void LibVLC::fullscreen(bool fullscreen)
{
	libvlc_set_fullscreen(libVlcMediaPlayer, fullscreen);
}

void LibVLC::pause()
{
	libvlc_media_player_pause(libVlcMediaPlayer);
}

void LibVLC::resume()
{
    libvlc_media_player_pause( libVlcMediaPlayer );
}

libvlc_time_t LibVLC::getLength()
{
	return libvlc_media_player_get_length(libVlcMediaPlayer);
}

libvlc_time_t LibVLC::getDuration()
{
	return libvlc_media_get_duration(libVlcMediaItem);
}

int LibVLC::getWidth()
{
	return libvlc_video_get_width(libVlcMediaPlayer);
}

int LibVLC::getHeight()
{
	return libvlc_video_get_height(libVlcMediaPlayer);
}

int LibVLC::isPlaying()
{
	return libvlc_media_player_is_playing(libVlcMediaPlayer);
}

void LibVLC::setRepeat(int numRepeats)
{
	repeat = numRepeats;
/*	if (libVlcMediaItem!=nullptr)
	{
		std::string sa = "input-repeat=";
		sa += std::to_string(repeat);
		//libvlc_media_add_option(libVlcMediaItem, sa.c_str() );	
		if (repeat==-1)
			libvlc_media_add_option(libVlcMediaItem, "input-repeat=-1" );	
		else if (repeat==0)
			libvlc_media_add_option(libVlcMediaItem, "input-repeat=0" );	
		//std::cout << "Num repeats: " << sa << std::endl;
	}
	*/
}

int LibVLC::getRepeat()
{
	return repeat;
}

const char* LibVLC::getLastError()
{
	return libvlc_errmsg();	
}

void LibVLC::setVolume(float volume)
{
	if (volume>255)
		volume = 255.0;

	vol = volume;
	if (libVlcMediaPlayer!=NULL && libVlcMediaPlayer!=nullptr)
	{
		try
		{
			//libvlc_audio_set_volume(libVlcMediaPlayer, volume);
			libvlc_audio_set_volume(libVlcMediaPlayer, 255.0);
		}
		catch(int e)
		{
		}
	}
}

float LibVLC::getVolume()
{
    float volume = libvlc_audio_get_volume( libVlcMediaPlayer );
    return volume;
}

libvlc_time_t LibVLC::getTime()
{
	if (libVlcMediaPlayer!=NULL && libVlcMediaPlayer!=nullptr)
	{
		try
		{
			int64_t t = libvlc_media_player_get_time( libVlcMediaPlayer );
			return t;
		}
		catch(int e)
		{
			return 0;
		}
	}
	else
		return 0;
}

void LibVLC::setTime(libvlc_time_t time)
{
	libvlc_media_player_set_time(libVlcMediaPlayer, time);
}

float LibVLC::getPosition()
{
    return libvlc_media_player_get_position( libVlcMediaPlayer );
}

void LibVLC::setPosition(float pos)
{
	libvlc_media_player_set_position(libVlcMediaPlayer, pos);
}

bool LibVLC::isSeekable()
{
    return ( libvlc_media_player_is_seekable( libVlcMediaPlayer ) == 1 );
}

void LibVLC::openMedia(const char* mediaPathName)
{
	libVlcMediaItem = libvlc_media_new_location(libVlcInstance, mediaPathName);
	//libVlcMediaItem = libvlc_media_new_path(libVlcInstance, mediaPathName);
    libvlc_media_player_set_media(libVlcMediaPlayer, libVlcMediaItem);    
}

//void MediaPlayer::setMedia( Media* media )
//{
    //libvlc_media_player_set_media( m_internalPtr, media->getInternalPtr() );
//}

//void
//MediaPlayer::getSize( quint32 *outWidth, quint32 *outHeight )
//{
    //libvlc_video_get_size( m_internalPtr, 0, outWidth, outHeight );
//}

float LibVLC::getFPS()
{
    return libvlc_media_player_get_fps( libVlcMediaPlayer );
}

void LibVLC::nextFrame()
{
    libvlc_media_player_next_frame( libVlcMediaPlayer );
}

bool LibVLC::hasVout()
{
    return libvlc_media_player_has_vout( libVlcMediaPlayer );
}
/*
void LibVLC::setXwindow(uint32_t drawable)
{
	libvlc_media_player_set_xwindow(*this, drawable);
}

uint32_t LibVLC::xwindow()
{
	return libvlc_media_player_get_xwindow(*this);
}

void LibVLC::setHwnd(void * drawable)
{
	libvlc_media_player_set_hwnd(*this, drawable);
}

void* LibVLC::hwnd()
{
	return libvlc_media_player_get_hwnd(*this);
}*/

/////////////////////////////////////////////////////////////////////////////////////

void LibVLC::useHWacceleration(bool hwAcc)
{
	if (hwAcc)
	{
		//libvlc_media_add_option(libVlcMediaItem, ":hwdec=vaapi");
		//libvlc_media_add_option(libVlcMediaItem, ":ffmpeg-hw");
		//libvlc_media_add_option(libVlcMediaItem, ":avcodec-hw=dxva2.lo");
		//libvlc_media_add_option(libVlcMediaItem, ":avcodec-hw=any");
		//libvlc_media_add_option(libVlcMediaItem, ":avcodec-hw=dxva2");
		//libvlc_media_add_option(libVlcMediaItem, "--avcodec-hw=dxva2");
		//libvlc_media_add_option(libVlcMediaItem, ":avcodec-hw=vaapi");
	}
}

/////////////////////////////////////////////////////////////////////////////////////

void LibVLC::registerEvents()
{
    libvlc_event_attach( eventManager, libvlc_MediaPlayerPlaying,         callbacks, this );
    libvlc_event_attach( eventManager, libvlc_MediaPlayerSnapshotTaken,   callbacks, this );
    libvlc_event_attach( eventManager, libvlc_MediaPlayerTimeChanged,     callbacks, this );
    libvlc_event_attach( eventManager, libvlc_MediaPlayerPlaying,         callbacks, this );
    libvlc_event_attach( eventManager, libvlc_MediaPlayerPaused,          callbacks, this );
    libvlc_event_attach( eventManager, libvlc_MediaPlayerStopped,         callbacks, this );
    libvlc_event_attach( eventManager, libvlc_MediaPlayerEndReached,      callbacks, this );
    libvlc_event_attach( eventManager, libvlc_MediaPlayerPositionChanged, callbacks, this );
    libvlc_event_attach( eventManager, libvlc_MediaPlayerLengthChanged,   callbacks, this );
    libvlc_event_attach( eventManager, libvlc_MediaPlayerEncounteredError,callbacks, this );
    libvlc_event_attach( eventManager, libvlc_MediaPlayerPausableChanged, callbacks, this );
    libvlc_event_attach( eventManager, libvlc_MediaPlayerSeekableChanged, callbacks, this );
}

//cpp::Function<Void (String)> vlcCallbackMth;
//int LibVLC::callbackIndex;

void LibVLC::callbacks( const libvlc_event_t* event, void* ptr )
{
    LibVLC* self = reinterpret_cast<LibVLC*>( ptr );
	
	//String msg = String("");
	//std::string msg = "";
	
    switch ( event->type )
    {
		case libvlc_MediaPlayerPlaying:
			//cout << "playing" << endl;
			//msg = "LibVLC::playing";
			self->flags[1]=1;
			self->setInitProps();
			break;
		case libvlc_MediaPlayerPaused:
			//msg = "LibVLC::paused";
			self->flags[2]=1;
			break;
		case libvlc_MediaPlayerStopped:
			//msg = "LibVLC::stopped";
			self->flags[3]=1;
			break;
		case libvlc_MediaPlayerEndReached:
			//msg = "LibVLC::endReached";
			self->flags[4]=1;
			break;
		case libvlc_MediaPlayerTimeChanged:
			//msg = String("LibVLC::timeChangeed");
			//self->emit timeChanged( event->u.media_player_time_changed.new_time );
			self->flags[5]=event->u.media_player_time_changed.new_time;
			break;
		case libvlc_MediaPlayerPositionChanged:
			//msg = String("LibVLC::positionChanged");
			//self->emit positionChanged( event->u.media_player_position_changed.new_position );
			self->flags[6]=event->u.media_player_position_changed.new_position;
			break;
		case libvlc_MediaPlayerLengthChanged:
			//msg = String("LibVLC::lengthChanged");
			//self->emit lengthChanged( event->u.media_player_length_changed.new_length );
			self->flags[7]=event->u.media_player_length_changed.new_length;
			break;
		case libvlc_MediaPlayerSnapshotTaken:
			//msg = String("LibVLC::snapshotTaken");
			//self->emit snapshotTaken( event->u.media_player_snapshot_taken.psz_filename );
			//flags[8]=event->u.media_player_length_changed.new_length;
			break;
		case libvlc_MediaPlayerEncounteredError:
			//msg = "LibVLC::error";
			//qDebug() << '[' << (void*)self << "] libvlc_MediaPlayerEncounteredError received."
					//<< "This is not looking good...";
			self->flags[9]=1;
			
			//RaiseException(0x0000DEAD,0,0,0);
			
			break;
		case libvlc_MediaPlayerSeekableChanged:
			//msg = String("LibVLC::seekableChanged");
			//self->emit volumeChanged();
			self->flags[10]=1;
			break;
		case libvlc_MediaPlayerPausableChanged:
		case libvlc_MediaPlayerTitleChanged:
		case libvlc_MediaPlayerNothingSpecial:
		case libvlc_MediaPlayerOpening:
			//msg = "LibVLC::opening";
			self->flags[11]=1;
			break;
		case libvlc_MediaPlayerBuffering:
			//msg = "LibVLC::buffering";
			self->flags[12]=1;
			break;
		case libvlc_MediaPlayerForward:
			self->flags[13]=1;
			break;
		case libvlc_MediaPlayerBackward:
			self->flags[14]=1;
			break;
		default:
	//        qDebug() << "Unknown mediaPlayerEvent: " << event->type;
			break;
    }
	
	//if (msg!=String(""))
	//{
		//self->vlcCallbackMth(msg + String("_$") + String(self->callbackIndex));
	//}
}

/////////////////////////////////////////////////////////////////////////////////////

//void LibVLC::setCallback(cpp::Function<void (String)> callback, int cbIndex)
//{
	//callbackIndex = cbIndex;
	//vlcCallbackMth = callback;
//}

/////////////////////////////////////////////////////////////////////////////////////

/*

libvlc_media_get_state(libvlc_media_t *p_md)	


void
MediaPlayer::takeSnapshot( const char* outputFile, unsigned int width, unsigned int heigth )
{
    libvlc_video_take_snapshot( *this, 0, outputFile, width, heigth );
}

bool
MediaPlayer::isSeekable()
{
    return ( libvlc_media_player_is_seekable( m_internalPtr ) == 1 );
}

void
MediaPlayer::setDrawable( void* drawable )
{
#if defined ( Q_WS_MAC )
    libvlc_media_player_set_nsobject( m_internalPtr, drawable );
#elif defined ( Q_OS_UNIX )
    libvlc_media_player_set_xwindow( m_internalPtr, reinterpret_cast<intptr_t>( drawable ) );
#elif defined ( Q_OS_WIN )
    libvlc_media_player_set_hwnd( m_internalPtr, drawable );
#endif
}

void
MediaPlayer::setMedia( Media* media )
{
    libvlc_media_player_set_media( m_internalPtr, media->getInternalPtr() );
}

void
MediaPlayer::getSize( quint32 *outWidth, quint32 *outHeight )
{
    libvlc_video_get_size( m_internalPtr, 0, outWidth, outHeight );
}

float
MediaPlayer::getFps()
{
    return libvlc_media_player_get_fps( m_internalPtr );
}

void
MediaPlayer::nextFrame()
{
    libvlc_media_player_next_frame( m_internalPtr );
}

bool
MediaPlayer::hasVout()
{
    return libvlc_media_player_has_vout( m_internalPtr );
}

const QString&
MediaPlayer::getLoadedFileName() const
{
    return m_media->getFileName();
}

QString
MediaPlayer::getLoadedMRL()
{
    Media::internalPtr     media = libvlc_media_player_get_media( m_internalPtr );
    char* str = libvlc_media_get_mrl( media );
    return QString( str );
}

int
MediaPlayer::getNbAudioTrack()
{
    int res = libvlc_audio_get_track_count( m_internalPtr );
    return res;
}

int
MediaPlayer::getNbVideoTrack()
{
    int res = libvlc_video_get_track_count( m_internalPtr );
    return res;
}

void
MediaPlayer::setKeyInput( bool enabled )
{
    libvlc_video_set_key_input( m_internalPtr, enabled );
}
*/





//char *str LibVLC::getMeta(libvlc_meta_t meta)
//{
	//return libvlc_media_get_meta(libVlcMediaPlayer, meta);
//}
/*
  vlcMetaPrint("Artist", media, libvlc_meta_Artist);
    vlcMetaPrint("Title", media, libvlc_meta_Title);
    vlcMetaPrint("Genre", media, libvlc_meta_Genre);
    vlcMetaPrint("Copyright", media, libvlc_meta_Copyright);
    vlcMetaPrint("Album", media, libvlc_meta_Album);
    vlcMetaPrint("Track no:", media, libvlc_meta_TrackNumber);
    vlcMetaPrint("Description", media, libvlc_meta_Description);
    vlcMetaPrint("Rating", media, libvlc_meta_Rating);
    vlcMetaPrint("Date", media, libvlc_meta_Date);
    vlcMetaPrint("Setting", media, libvlc_meta_Setting);
    vlcMetaPrint("URL", media, libvlc_meta_URL);
    vlcMetaPrint("Language", media, libvlc_meta_Language);
    // FIXME: Is this present on any file metadata?
    // vlcMetaPrint("", media, libvlc_meta_NowPlaying);
    vlcMetaPrint("Publisher", media, libvlc_meta_Publisher);
    vlcMetaPrint("EncodedBy", media, libvlc_meta_EncodedBy);
    vlcMetaPrint("Artwork URL", media, libvlc_meta_ArtworkURL);
    vlcMetaPrint("Track ID", media, libvlc_meta_TrackID);
    vlcMetaPrint("Track total", media, libvlc_meta_TrackTotal);
    vlcMetaPrint("Director", media, libvlc_meta_Director);
    vlcMetaPrint("Season", media, libvlc_meta_Season);
    vlcMetaPrint("Episode", media, libvlc_meta_Episode);
    vlcMetaPrint("ShowName", media, libvlc_meta_ShowName);
    vlcMetaPrint("Actors", media, libvlc_meta_Actors);
*/

/*
libvlc_instance_t * LibVLC::createNew()
{
	return libvlc_new(0, NULL);
}

libvlc_media_t * LibVLC::mediaNewPath(libvlc_instance_t * inst, const char * path)
{
	return libvlc_media_new_path(inst, path);
}

libvlc_media_player_t * LibVLC::mediaPlayerNewFromMedia(libvlc_media_t *m)
{
	return libvlc_media_player_new_from_media(m);
}

void LibVLC::mediaRelease(libvlc_media_t *m)
{
	libvlc_media_release(m);
}

void LibVLC::mediaPlayerPlay(libvlc_media_player_t *mp)
{
	libvlc_media_player_play(mp);
}

void LibVLC::setFullscreen(libvlc_media_player_t *mp, bool fss)
{
	libvlc_set_fullscreen(mp, fss);
}*/

/////////////////////////////////////////////////////////////////////////////////////
/*
int bpp = 4;

typedef struct ctx
{
	unsigned char *pixeldata;
	std::mutex imagemutex;
} t_ctx;

t_ctx ctx;

static void *lock(void *data, void **p_pixels)
{
	t_ctx *ctx = (t_ctx*)data;

	ctx->imagemutex.lock();
	*p_pixels = ctx->pixeldata;

	uint8_t * pixel = (uint8_t *)ctx->pixeldata;
	const int pitch = 1920;
	const int height = 1080;

	return NULL;
}

static void unlock(void *data, void *id, void *const *p_pixels)
{
	t_ctx *ctx = (t_ctx *)data;

	ctx->imagemutex.unlock();

	//assert(id == NULL);
}

static void display(void *opaque, void *picture)
{
	int x = 0;
	x++;
}

libvlc_instance_t * inst;
libvlc_media_player_t *mp;
libvlc_media_t * m;

//===================================================================================
// Main 
//-----------------------------------------------------------------------------------		

void setup
{
	inst = libvlc_new(0, NULL);
	m = libvlc_media_new_path (inst, "test.mp4");
        
	mp = libvlc_media_player_new_from_media(m);
	libvlc_media_release (m);

	libvlc_media_player_play (mp);
}
*/

/*
void setup
{
	inst = libvlc_new(0, NULL);
	//m = libvlc_media_new_location (inst, "screen://");
	m = libvlc_media_new_path (inst, "test.mp4");
        
	/////////////////////////////////////////////////////////////////////////////////

	mp = libvlc_media_player_new_from_media(m);
	libvlc_media_release (m);
	t_ctx ctx = {};

	ctx.pixeldata = new unsigned char[1920 * 1080 * bpp];

	//libvlc_video_set_format(mp, "UYVY", 1920, 1080, 1920* bpp);
	libvlc_video_set_format(mp, "RV32", 1920, 1080, 1920 * bpp);
	//libvlc_video_set_format(mp, "RV24", 1920, 1080, 1920 * 3);

	//std::cout << "sbytes" << 1920 * 4 << "h" << 1080 << std::endl;

	libvlc_video_set_callbacks(mp, lock, unlock, display, &ctx);

	/////////////////////////////////////////////////////////////////////////////////

	libvlc_media_player_play (mp);

	/////////////////////////////////////////////////////////////////////////////////

	_sleep ((unsigned long)wait_time);

	length = libvlc_media_player_get_length(mp);
	width = libvlc_video_get_width(mp);
	height = libvlc_video_get_height(mp);
	//printf("Stream Duration: %ds\n",length/1000);
	//printf("Resolution: %d x %d\n",width,height);

	_sleep((unsigned long)(length-wait_time));

	/////////////////////////////////////////////////////////////////////////////////

	libvlc_media_player_stop (mp);
	libvlc_media_player_release (mp);
	libvlc_release (inst);
	delete[] ctx.pixeldata;
 
	fclose(fp);

	return 0;
}
*/
/////////////////////////////////////////////////////////////////////////////////////
/*
}
*/