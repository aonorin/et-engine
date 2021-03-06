/*
 * This file is part of `et engine`
 * Copyright 2009-2015 by Sergey Reznik
 * Please, modify content only if you know what are you doing.
 *
 */

#include <et/rendering/rendercontext.h>
#include <et/app/application.h>

using namespace et;

namespace et
{
	uint32_t randomInteger(uint32_t limit);
}

Application::Application()
{
	sharedObjectFactory();
	log::addOutput(log::ConsoleOutput::Pointer::create());

	_lastQueuedTimeMSec = queryContiniousTimeInMilliSeconds();
	
	threading::setMainThreadIdentifier(threading::currentThread());

	delegate()->setApplicationParameters(_parameters);

	platformInit();
	platformActivate();
	
	_backgroundThread.run();
}

IApplicationDelegate* Application::delegate()
{
	if (_delegate == nullptr)
	{
		_delegate = initApplicationDelegate();
		ET_ASSERT(_delegate);
		_identifier = _delegate->applicationIdentifier();
	}
    
	return _delegate;
}

int Application::run(int argc, char* argv[])
{
#if (ET_DEBUG)
#	if defined(ET_CONSOLE_APPLICATION)
		log::info("[et-engine] Version: %d.%d, running console application in debug mode.",
			ET_MAJOR_VERSION, ET_MINOR_VERSION);
#	else
		log::info("[et-engine] Version: %d.%d, running in debug mode.",
			ET_MAJOR_VERSION, ET_MINOR_VERSION);
#	endif
#endif
	
	for (int i = 0; i < argc; ++i)
		_launchParameters.push_back(argv[i]);

	return platformRun(argc, argv);
}

void Application::enterRunLoop()
{
	registerRunLoop(_runLoop);
	_running = true;

	if (_parameters.shouldPreserveRenderContext)
		_renderContext->pushAndActivateRenderingContext();
	
	_standardPathResolver.setRenderContext(_renderContext);
	delegate()->applicationDidLoad(_renderContext);
	
#if defined(ET_CONSOLE_APPLICATION)
	
	setActive(true);
	
	while (_running)
		performUpdateAndRender();
	
	terminated();
	
#else
	
	_renderContext->init();
	setActive(true);
	
#endif

	if (_parameters.shouldPreserveRenderContext)
		_renderContext->popRenderingContext();
}

void Application::exitRunLoop()
{
	unregisterRunLoop(_runLoop);
}

void Application::performRendering()
{
#if !defined(ET_CONSOLE_APPLICATION)
	_renderContext->beginRender();
	_delegate->render(_renderContext);
	_renderContext->endRender();
#endif
}

bool Application::shouldPerformRendering()
{
	uint64_t currentTime = queryContiniousTimeInMilliSeconds();
	uint64_t elapsedTime = currentTime - _lastQueuedTimeMSec;

	if (elapsedTime < _fpsLimitMSec)
	{
		uint64_t sleepInterval = (_fpsLimitMSec - elapsedTime) +
			(randomInteger(1000) > _fpsLimitMSecFractPart ? 0 : static_cast<uint64_t>(-1));
		
		threading::sleepMSec(sleepInterval);
		
		return false;
	}
	_lastQueuedTimeMSec = queryContiniousTimeInMilliSeconds();
	
	return !_suspended;
}

void Application::performUpdateAndRender()
{
	ET_ASSERT(_running && !_suspended);
	
	_runLoop.update(_lastQueuedTimeMSec);

#if !defined(ET_CONSOLE_APPLICATION)
	performRendering();
#endif
}

void Application::setFrameRateLimit(size_t value)
{
	_fpsLimitMSec = (value == 0) ? 0 : 1000 / value;
	_fpsLimitMSecFractPart = (value == 0) ? 0 : (1000000 / value - 1000 * _fpsLimitMSec);
}

void Application::setActive(bool active)
{
	if (!_running || (_active == active)) return;

	_active = active;
	
	if (active)
	{
		if (_suspended)
			resume();

		_delegate->applicationWillActivate();
		
		if (_postResizeOnActivate)
		{
			_delegate->applicationWillResizeContext(_renderContext->sizei());
			_postResizeOnActivate = false;
		}
		
		platformActivate();
	}
	else
	{
		_delegate->applicationWillDeactivate();
		platformDeactivate();
		
		if (_parameters.shouldSuspendOnDeactivate)
			suspend();
	}
}

void Application::contextResized(const vec2i& size)
{
	if (_running)
	{
		if (_active)
			_delegate->applicationWillResizeContext(size);
		else
			_postResizeOnActivate = true;
	}
}

void Application::suspend()
{
	if (_suspended) return;

	delegate()->applicationWillSuspend();
	_runLoop.pause();

	platformSuspend();
	
	_suspended = true;
}

void Application::resume()
{
	ET_ASSERT(_suspended && "Should be suspended.");

	delegate()->applicationWillResume();
	
	_suspended = false;

	platformResume();

	_lastQueuedTimeMSec = queryContiniousTimeInMilliSeconds();
	_runLoop.update(_lastQueuedTimeMSec);
	_runLoop.resume();
}

void Application::stop()
{
	_running = false;
}

void Application::terminated()
{
	_delegate->applicationWillTerminate();
	stop();
}

std::string Application::resolveFileName(const std::string& path)
{
	std::string result;
	
	if (_customPathResolver.valid())
		result = _customPathResolver->resolveFilePath(path);
	
	if (!fileExists(result))
		result = _standardPathResolver.resolveFilePath(path);
	
	return fileExists(result) ? result : path;
}

std::string Application::resolveFolderName(const std::string& path)
{
	std::string result;
	
	if (_customPathResolver.valid())
		result = _customPathResolver->resolveFolderPath(path);
	
	if (!folderExists(result))
		result = _standardPathResolver.resolveFolderPath(path);
	
	return folderExists(result) ? result : path;
}

std::set<std::string> Application::resolveFolderNames(const std::string& path)
{
	std::set<std::string> result;
	
	if (_customPathResolver.valid())
		result = _customPathResolver->resolveFolderPaths(path);
	
	auto standard = _standardPathResolver.resolveFolderPaths(path);
	result.insert(standard.begin(), standard.end());
	
	return result;
}

void Application::pushSearchPath(const std::string& path)
{
	_standardPathResolver.pushSearchPath(path);
}

void Application::pushRelativeSearchPath(const std::string& path)
{
	_standardPathResolver.pushRelativeSearchPath(path);
}

void Application::pushSearchPaths(const std::set<std::string>& paths)
{
	_standardPathResolver.pushSearchPaths(paths);
}

void Application::popSearchPaths(size_t amount)
{
	_standardPathResolver.popSearchPaths(amount);
}

void Application::setPathResolver(PathResolver::Pointer resolver)
{
	_customPathResolver = resolver;
}

void Application::setShouldSilentPathResolverErrors(bool e)
{
	_standardPathResolver.setSilentErrors(e);
}

const ApplicationIdentifier& Application::identifier() const
{
	return _identifier;
}


/*
 * Service
 */

namespace
{
	static std::map<threading::ThreadIdentifier, RunLoop*> allRunLoops;
	bool removeRunLoopFromMap(RunLoop* ptr)
	{
		bool found = false;
		auto i = allRunLoops.begin();
		while (i != allRunLoops.end())
		{
			if (i->second == ptr)
			{
				found = true;
				i = allRunLoops.erase(i);
			}
			else
			{
				++i;
			}
		}
		return found;
	}
}

RunLoop& et::mainRunLoop()
{
	return application().mainRunLoop();
}

RunLoop& et::backgroundRunLoop()
{
	return application().backgroundRunLoop();
}

RunLoop& et::currentRunLoop()
{
	auto threadId = threading::currentThread();
	
	if (allRunLoops.count(threadId) > 0)
		return mainRunLoop();
	
	return *(allRunLoops.at(threadId));
}

TimerPool::Pointer& et::mainTimerPool()
{
	return application().mainRunLoop().firstTimerPool();
}

TimerPool::Pointer et::currentTimerPool()
{
	return currentRunLoop().firstTimerPool();
}

void et::registerRunLoop(RunLoop& runLoop)
{
	auto currentThread = threading::currentThread();
	ET_ASSERT(allRunLoops.count(currentThread) == 0);

	removeRunLoopFromMap(&runLoop);
	allRunLoops.insert({currentThread, &runLoop});
}

void et::unregisterRunLoop(RunLoop& runLoop)
{
	auto success = removeRunLoopFromMap(&runLoop);
	if (!success)
	{
		log::error("Attempt to unregister non-registered RunLoop");
	}
}

const std::string et::kSystemEventType = "kSystemEventType";
const std::string et::kSystemEventRemoteNotification = "kSystemEventRemoteNotification";
const std::string et::kSystemEventRemoteNotificationStatusChanged = "kSystemEventRemoteNotificationStatusChanged";
const std::string et::kSystemEventOpenURL = "kSystemEventOpenURL";
