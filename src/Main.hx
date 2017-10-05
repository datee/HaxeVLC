package;

import openfl.display.Sprite;
import vlc.VlcBitmap;

class Main extends Sprite 
{
	/////////////////////////////////////////////////////////////////////////////////////
	
	public function new()
	{
		super();
		
		var vlcBitmap = new VlcBitmap();
		vlcBitmap.onVideoReady = onVlcVideoReady;
		vlcBitmap.onPlay = onVLCPlay;
		vlcBitmap.onPause = onVLCPause;
		vlcBitmap.onResume = onVLCResume;
		vlcBitmap.onStop = onVLCStop;
		vlcBitmap.onSeek = onVLCSeek;
		vlcBitmap.onComplete = onVLCComplete;
		vlcBitmap.onProgress = onVLCProgress;
		vlcBitmap.volume = 1;
		vlcBitmap.repeat = -1;
		addChild(vlcBitmap);
		
		vlcBitmap.inWindow = false;
		vlcBitmap.fullscreen = true;
		
		vlcBitmap.play(checkFile("SampleVideo.mp4"));
	}
	
	/////////////////////////////////////////////////////////////////////////////////////

	function checkFile(fileName:String):String 
	{
		var pDir = "";
		var appDir = "file:///" + Sys.getCwd() + "/";
		if (fileName.indexOf(":")==-1) //Not a path
			pDir = appDir;
		else if (fileName.indexOf("file://") ==-1 || fileName.indexOf("http") ==-1) //C:, D: etc? ..missing "file:///" ?
			pDir = "file:///";
			
		return pDir+fileName;
	}
	
	/////////////////////////////////////////////////////////////////////////////////////

	function onVlcVideoReady() 
	{
	}
	
	function onVLCPlay() 
	{
	}
	
	function onVLCPause() 
	{
	}
	
	function onVLCResume() 
	{
	}
	
	function onVLCStop() 
	{
	}
	
	function onVLCSeek() 
	{
	}
	
	function onVLCComplete() 
	{
	}
	
	function onVLCProgress() 
	{
	}
	
	/////////////////////////////////////////////////////////////////////////////////////
}
