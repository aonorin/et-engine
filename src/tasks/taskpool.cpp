/*
 * This file is part of `et engine`
 * Copyright 2009-2014 by Sergey Reznik
 * Please, do not modify content without approval.
 *
 */

#include <algorithm>
#include <et/tasks/taskpool.h>

using namespace et;

TaskPool::TaskPool()
{
}

TaskPool::~TaskPool() 
{
	CriticalSectionScope lock(_csModifying);
	
	for (auto i : _tasks)
		sharedObjectFactory().deleteObject(i);
}

void TaskPool::addTask(Task* t, float delay)
{
	CriticalSectionScope lock(_csModifying);
	
	auto alreadyAdded = std::find(_tasksToAdd.begin(), _tasksToAdd.end(), t);
	if (alreadyAdded == _tasksToAdd.end())
	{
		t->setExecutionTime(_lastTime + delay);
		_tasksToAdd.push_back(t); 
	}
}

void TaskPool::update(float currentTime)
{
	joinTasks();
	
	_lastTime = currentTime;
	
	auto i = _tasks.begin();
	while (i != _tasks.end())
	{
		Task* task = (*i);
		if (_lastTime >= task->executionTime())
		{
			task->execute();
			sharedObjectFactory().deleteObject(task);
			i = _tasks.erase(i);
		}
		else
		{
			++i;
		}
	}
}

bool TaskPool::hasTasks()
{
	CriticalSectionScope lock(_csModifying);
	return !(_tasks.empty() && _tasksToAdd.empty());
}

void TaskPool::joinTasks()
{
	CriticalSectionScope lock(_csModifying);
	
	if (!_tasksToAdd.empty())
	{
		_tasks.insert(_tasks.end(), _tasksToAdd.begin(), _tasksToAdd.end());
		_tasksToAdd.clear();
	}
}
