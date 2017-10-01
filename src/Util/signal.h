#pragma once
#include <stdint.h>
#include <vector>
#include <functional>
#include <algorithm>

template <typename ...T> class CSignal
{
	public:
		void connect(intptr_t id, std::function <void(T...)> func)
		{
			auto iter = std::find_if(m_observers.begin(), m_observers.end(),
				[id] (const Observer& obs) -> bool
				{
					return obs.id == id;
				}
			);

			if (iter != m_observers.end())
			{
				iter->func = func;
			}
			else
			{
				Observer newObs = {id, func};
				m_observers.push_back(newObs);
			}
		}

		void connect(void* id, std::function <void(T...)> func)
		{
			connect(reinterpret_cast <uintptr_t> (id), func);
		}

		void disconnect(intptr_t id)
		{
			m_observers.erase(std::remove_if(m_observers.begin(), m_observers.end(),
			[id] (const Observer& obs) -> bool
			{
				return obs.id == id;
			}), m_observers.end());
		}

		void disconnect(void* id)
		{
			disconnect(reinterpret_cast <uintptr_t> (id));
		}

		void operator () (T... args)
		{
			// by iterating in reverse, we guarantee that any observer that
			// wants to disconnect during the signal, can do so
			for (int i = static_cast<int> (m_observers.size()) - 1; i >=0; --i)
			{
				m_observers[i].func(args...);
			}
		}

	private:
		struct Observer
		{
			intptr_t id;
			std::function <void(T...)> func;
		};

		std::vector <Observer> m_observers;
};
