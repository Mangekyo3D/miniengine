#pragma once
#include <stdint.h>
#include <vector>
#include <functional>
#include <algorithm>

template <typename ...Args> class CSignal
{
	public:
		void connect(intptr_t id, std::function <void(Args...)> func)
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

		void connect(void* id, std::function <void(Args...)> func)
		{
			connect(reinterpret_cast <uintptr_t> (id), func);
		}

		template <typename T>
		void connect(T *id, void (T::*func)(Args...)) {
			connect(reinterpret_cast <uintptr_t> (id), [=](Args... args) {
				(id->*func)(args...);
			});
		}

		template <typename T>
		void connect(T *id, void (T::*func)(Args...) const) {
			connect(reinterpret_cast <uintptr_t> (id), [=](Args... args) {
				(id->*func)(args...);
			});
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

		void operator () (Args... args)
		{
			// by iterating in reverse, we guarantee that any observer that
			// wants to disconnect during the signal, can do so
			for (int i = static_cast<int> (m_observers.size()) - 1; i >=0; --i)
			{
				m_observers[i].func(std::forward<Args>(args)...);
			}
		}

	private:
		struct Observer
		{
			intptr_t id;
			std::function <void(Args...)> func;
		};

		std::vector <Observer> m_observers;
};
