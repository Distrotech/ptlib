/*
 * sound_directsound.h
 *
 * DirectX Sound driver implementation.
 *
 * Portable Windows Library
 *
 * Copyright (c) 2006-2007 Novacom, a division of IT-Optics
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.0 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
 * the License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is Portable Windows Library.
 *
 * The Initial Developer of the Original DirectSound Code is 
 * Vincent Luba <vincent.luba@novacom.be>
 *
 * Contributor(s): Ted Szoczei, Nimajin Software Consulting
 *
 * $Revision: 25079 $
 * $Author: rjongbloed $
 * $Date: 2011-01-20 12:23:32 -0600 (Thu, 20 Jan 2011) $
 */

#ifndef __DIRECTSOUND_H__
#define __DIRECTSOUND_H__


#include <ptlib.h>
#include <ptbuildopts.h>

#if defined(P_DIRECTSOUND)

#include <ptlib/sound.h>

#include <dsound.h>

#ifdef _WIN32_WCE
#define LPDIRECTSOUND8 LPDIRECTSOUND
#define LPDIRECTSOUNDBUFFER8 LPDIRECTSOUNDBUFFER
#define LPDIRECTSOUNDCAPTURE8 LPDIRECTSOUNDCAPTURE
#define LPDIRECTSOUNDCAPTUREBUFFER8 LPDIRECTSOUNDCAPTUREBUFFER
#define DirectSoundCreate8 DirectSoundCreate
#define IID_IDirectSoundBuffer8 IID_IDirectSoundBuffer
#define DirectSoundCaptureCreate8 DirectSoundCaptureCreate
#define IID_IDirectSoundCaptureBuffer8 IID_IDirectSoundCaptureBuffer
#endif

/** Usage
    1. instantiate with audio format or instantiate and call Open
	2. SetBuffers
	3. Read/Write
	4. destroy or Close
  */

class PSoundChannelDirectSound: public PSoundChannel
{
public:
  /**@name Construction */
  //@{
  /** Initialise with no device
   */
  PSoundChannelDirectSound();

  /** Initialise and open device
    */
  PSoundChannelDirectSound(const PString &device,
			     PSoundChannel::Directions dir,
			     unsigned numChannels,
			     unsigned sampleRate,
			     unsigned bitsPerSample);

  ~PSoundChannelDirectSound();
  //@}

  /** Provides a list of detected devices human readable names
      Returns the names array of enumerated devices as PStringArray
   */
  static PStringArray GetDeviceNames(PSoundChannel::Directions);

  /** Open a device with format specifications
      Device name corresponds to Multimedia name (first 32 characters)
      Device is prepared for operation, but not activated yet (no I/O
	  buffer - call SetBuffers for that).
	  Or you can use PlaySound or PlayFile - they call SetBuffers themselves)
    */
  PBoolean Open(const PString & device,
            Directions dir,
            unsigned numChannels,
            unsigned sampleRate,
            unsigned bitsPerSample);

  PString GetName() const { return deviceName; }

  PBoolean IsOpen() const
  {
    return (mDirection == Player && sAudioPlaybackDevice != NULL) || sAudioCaptureDevice != NULL;
  }

  /** Stop I/O and destroy I/O buffer
   */
  PBoolean Abort();

  /** Destroy device
   */
  PBoolean Close();

  /** Change the audio format
      Resets I/O
    */
  PBoolean SetFormat(unsigned numChannels,
                 unsigned sampleRate,
                 unsigned bitsPerSample);

  unsigned GetChannels() const { return mNumChannels; }
  unsigned GetSampleRate() const { return mSampleRate; }
  unsigned GetSampleSize() const { return mBitsPerSample; }

  /** Configure the device's transfer buffers.
      No audio can be played or captured until after this method is set!
	  (PlaySound and PlayFile can be used though - they call here.)
      Read and write functions wait for input or space (blocking thread)
	  in increments of buffer size.
	  Best to make size the same as the len to be given to Read or Write.
      Best performance requires count of 4
      Resets I/O
    */
  PBoolean SetBuffers(PINDEX size, PINDEX count);
  PBoolean GetBuffers(PINDEX & size, PINDEX & count);

  /** Write specified number of bytes from buf to playback device
      Blocks thread until all bytes have been transferred to device
    */
  PBoolean Write(const void * buf, PINDEX len);

  /** Read specified number of bytes from capture device into buf
      Number of bytes actually read is a multiple of format frame size
	  Blocks thread until number of bytes have been received
    */
  PBoolean Read(void * buf, PINDEX len);

  /** Resets I/O, changes audio format to match sound and configures the 
      device's transfer buffers into one huge buffer, into which the entire
	  sound is loaded and started playing.
	  Returns immediately when wait is false, so you can do other stuff while
	  sound plays.
    */
  PBoolean PlaySound(const PSound & sound, PBoolean wait);

  /** Resets I/O, changes audio format to match file and reconfigures the
      device's transfer buffers. Accepts .wav files. Plays audio from file in
	  1/2 second chunks. Wait refers to waiting for completion of last chunk.
    */
  PBoolean PlayFile(const PFilePath & filename, PBoolean wait);

  /** Checks space available for writing audio to play.
	  Returns true if space enough for one buffer as set by SetBuffers.
	  Sets 'available' member for use by Write.
    */
  PBoolean IsPlayBufferFree();

  /** Repeatedly checks until there's space to fit buffer.
      Yields thread between checks.
	  Loop can be ended by calling Abort()
    */
  PBoolean WaitForPlayBufferFree();

  // all below are untested

  PBoolean HasPlayCompleted();
  PBoolean WaitForPlayCompletion();

  PBoolean RecordSound(PSound & sound);
  PBoolean RecordFile(const PFilePath & filename);
  PBoolean StartRecording();
  PBoolean IsRecordBufferFull();
  PBoolean AreAllRecordBuffersFull();
  PBoolean WaitForRecordBufferFull();
  PBoolean WaitForAllRecordBuffersFull();

  PBoolean SetVolume (unsigned);
  PBoolean GetVolume (unsigned &);

private:

  unsigned mNumChannels;// 1=mono, 2=stereo, etc.
  unsigned mSampleRate;
  unsigned mBitsPerSample;
  
  PString deviceName;
  Directions mDirection;

  LPDIRECTSOUNDCAPTURE8 sAudioCaptureDevice;
  LPDIRECTSOUNDCAPTUREBUFFER mAudioCaptureBuffer;

  LPDIRECTSOUND8 sAudioPlaybackDevice;
  LPDIRECTSOUNDBUFFER mAudioPlaybackBuffer;
  LPDIRECTSOUNDBUFFER mAudioPrimaryPlaybackBuffer;
  
  PBoolean InitPlaybackBuffer();
  PBoolean InitPlaybackDevice(GUID *pGUID);
  
  PBoolean InitCaptureBuffer();
  PBoolean InitCaptureDevice(GUID *pGUID);
  
  PBoolean GetDeviceID (PString deviceName, GUID *pGUID);

  /** Writes (len) bytes from the buffer (*buf) to (position) in DirectX sound device buffer
      position is a byte offset from start of DX buffer.
      Returns the number of bytes actually written
    */
  PINDEX WriteToDXBuffer(const void * buf, PINDEX len, DWORD position);

  /** Reads (len) bytes from the buffer (*buf) from (position) in DirectX sound capture device buffer
      position is a byte offset from start of DX buffer.
      Returns the number of bytes actually read
    */
  PINDEX ReadFromDXBuffer(const void * buf, PINDEX len, DWORD position);
  
  PBoolean SetFormat ();

  PBoolean mStreaming;
  PINDEX mBufferSize;
  PINDEX mDXBufferSize;
  PINDEX mBufferCount;
  DWORD bufferByteOffset;   // byte offset from start of DX buffer to where we can write or read
  DWORD available;          // number of bytes space available to write, or available to read

  PINDEX mVolume;

  WAVEFORMATEX mWFX;        // audio format supplied to DirectSound
  HANDLE notificationEvent[2];  // [0]triggered by DirectSound at buffer boundaries, [1]by Close
  
  PMutex bufferMutex;
};

#endif // P_DIRECTSOUND

#endif  /* __DIRECTSOUND_H__ */
