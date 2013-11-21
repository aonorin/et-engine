/*
 * This file is part of `et engine`
 * Copyright 2009-2013 by Sergey Reznik
 * Please, do not modify content without approval.
 *
 */

#include <et/app/application.h>
#include <et/sound/sound.h>

namespace et
{
    namespace audio
    {
        class PlayerPrivate
        {
		public:     
			ALuint source = 0;
		};
    }
}

using namespace et;
using namespace et::audio;

Player::Player() : 
	_private(new PlayerPrivate), _volumeAnimator(mainTimerPool())
{
	init();
}

Player::Player(Track::Pointer track) : 
	_private(new PlayerPrivate), _volumeAnimator(mainTimerPool())
{
	init();
	linkTrack(track);
}

Player::~Player()
{
	stop();

	alDeleteSources(1, &_private->source);
    checkOpenALError("alDeleteSources");

	delete _private;
}

void Player::init()
{
	ET_CONNECT_EVENT(_volumeAnimator.updated, Player::onVolumeUpdated)

	alGenSources(1, &_private->source);
    checkOpenALError("alGenSources");

	alSourcef(_private->source, AL_PITCH, 1.0f);
    checkOpenALError("alSourcef(..., AL_PITCH, ...)");

	setVolume(1.0f);
	
	vec3 nullVector;
	alSourcefv(_private->source, AL_POSITION, nullVector.data());
    checkOpenALError("alSourcefv(..., AL_POSITION, ...)");
	
	alSourcefv(_private->source, AL_VELOCITY, nullVector.data());
    checkOpenALError("alSourcefv(..., AL_VELOCITY, ...)");
}

void Player::play(bool looped)
{
	alSourcei(_private->source, AL_LOOPING, looped ? AL_TRUE : AL_FALSE);
    checkOpenALError("alSourcei(..., AL_LOOPING, ...)");
	
	alSourcePlay(_private->source);
    checkOpenALError("alSourcePlay");
}

void Player::play(Track::Pointer track, bool looped)
{
	linkTrack(track);
	play(looped);
}

void Player::pause()
{
	alSourcePause(_private->source);
    checkOpenALError("alSourcePause");
}

void Player::stop()
{
	retain();
	manager().streamingThread().removePlayer(Player::Pointer(this));
	release();
	
	alSourceStop(_private->source);
    checkOpenALError("alSourceStop");
}

void Player::rewind()
{
    alSourceRewind(_private->source);
    checkOpenALError("alSourceRewind");
}

void Player::linkTrack(Track::Pointer track)
{
    stop();
    
	_track = track;
	_track->rewind();
	_track->preloadBuffers();
	
	if (_track->streamed())
	{
		alSourceQueueBuffers(_private->source, _track->buffersCount(), _track->buffers());
		checkOpenALError("alSourceQueueBuffers(.., %d, %u)", _track->buffersCount(), _track->buffers());
		
		retain();
		manager().streamingThread().addPlayer(Player::Pointer(this));
		release();
	}
	else
	{
		alSourcei(_private->source, AL_BUFFER, _track->buffer());
		checkOpenALError("alSourcei(.., AL_BUFFER, ...)");
	}
}

void Player::setVolume(float value, float duration)
{
	_volumeAnimator.cancelUpdates();
	
	if (duration == 0.0f)
	{
		setVolume(value);
	}
	else
	{
		_volumeAnimator.animate(&_volume, _volume, value, duration);
	}
}

float Player::position() const
{
	if (_track.invalid()) return 0.0f;

	float sampleOffset = 0.0f;
	alGetSourcef(_private->source, AL_SAMPLE_OFFSET, &sampleOffset);
	checkOpenALError("alGetSourcef(..., AL_SAMPLE_OFFSET, ");

	return sampleOffset / static_cast<float>(_track->sampleRate());
}

bool Player::playing() const
{
	if (_track.invalid()) return false;
	
	ALint state = 0;
	alGetSourcei(_private->source, AL_SOURCE_STATE, &state);
	
	return (state == AL_PLAYING);
}

void Player::setPan(float pan)
{
	if (_track.invalid()) return;
	
	if (_track->channels() > 1)
	{
		log::warning("Unable to set pan for stereo sound: %s", _track->origin().c_str());
	}
	else
	{
		alSource3f(_private->source, AL_POSITION, pan, 0.0f, 0.0f);
		checkOpenALError("alSource3f(..., AL_POSITION, ");
	}
}

unsigned int Player::source() const
{
	return _private->source;
}

bool Player::loadNextBuffers(int processed, int remaining)
{
	while (processed > 0)
	{
		ALuint buffer = 0;
		alSourceUnqueueBuffers(_private->source, 1, &buffer);
		checkOpenALError("alSourceUnqueueBuffers");
		
		buffer = _track->loadNextBuffer();
		if (buffer > 0)
		{
			alSourceQueueBuffers(_private->source, 1, &buffer);
			checkOpenALError("alSourceQueueBuffers");
		}
		
		--processed;
	}
		
	return true;
}

void Player::onVolumeUpdated()
{
	setVolume(_volume);
}

void Player::setVolume(float v)
{
	static const float exponentFactor = std::log(0.001f);

	_volume = etMax(0.0f, etMin(1.0f, v));
	alSourcef(_private->source, AL_GAIN, std::exp(exponentFactor * (1.0f - _volume)));
	
	checkOpenALError("alSourcei(.., AL_GAIN, ...)");
}
