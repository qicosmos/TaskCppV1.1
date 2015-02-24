#pragma once
#include <vector>
#include <map>
#include <string>
#include <algorithm>
#include <future>
using namespace std;
#include <Any.hpp>
#include <Variant.hpp>
#include "Task.hpp"

namespace Cosmos
{
	class TaskGroup
	{
	public:

		TaskGroup()
		{
		}
		~TaskGroup()
		{
		}

		template<typename R, typename = typename std::enable_if<!std::is_same<R, void>::value>::type>
		void Run(Task<R()>& task)
		{
			m_group.insert(std::make_pair<RetVariant, Any>(R(), task.Run()));
		}

		void Run(Task<void()>& task)
		{
			m_voidGroup.push_back(task.Run());
		}

		template<typename R, typename = typename std::enable_if<!std::is_same<R, void>::value>::type>
		void Run(Task<R()>&& task)
		{
			m_group.insert(std::make_pair<RetVariant, Any>(R(), task.Run()));
		}

		void Run(Task<void()>&& task)
		{
			m_voidGroup.push_back(task.Run());
		}

		template<typename F>
		void Run(F&& f)
		{
			Run(typename Task<std::result_of<F()>::type()>(std::forward<F>(f)));
		}

		template<typename F, typename... Funs>
		void Run(F&& first, Funs&&... rest)
		{
			Run(std::forward<F>(first));
			Run(std::forward<Funs>(rest)...);
		}

		void Wait()
		{
			for (auto it = m_group.begin(); it != m_group.end(); ++it)
			{
				auto vrt = it->first;
				vrt.Visit([&](int a){FutureGet<int>(it->second); }, [&](double b){FutureGet<double>(it->second); },
					[&](string v){FutureGet<string>(it->second); }, [&](short v){FutureGet<short>(it->second); },
					[&](unsigned int v){FutureGet<unsigned int>(it->second); }
				);
			}

			for (auto it = m_voidGroup.begin(); it != m_voidGroup.end(); ++it)
			{
				it->get();
			}
		}

	private:
		template<typename T>
		void FutureGet(Any& f)
		{
			f.AnyCast<shared_future<T>>().get();
			//boost::any_cast<shared_future<T>>(f).get();
		}

		typedef Variant<int, double, string, short, unsigned int> RetVariant;
		multimap<RetVariant, Any> m_group;
		vector<std::shared_future<void>> m_voidGroup;
	};
}


