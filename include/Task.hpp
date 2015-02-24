#pragma once

#include <functional>
namespace Cosmos
{
	template<typename T>
	class Task;

	template<typename R, typename...Args>
	class Task<R(Args...)>
	{
		std::function<R(Args...)> m_fn;

	public:
		typedef R return_type;

		template<typename F>
		auto Then(F&& f)->Task<typename std::result_of<F(R)>::type(Args...)>
		{
			typedef typename std::result_of<F(R)>::type ReturnType;
			auto func = std::move(m_fn);
			return Task<ReturnType(Args...)>([func, &f](Args&&... args)
			{
				std::future<R> lastf = std::async(func, std::forward<Args>(args)...);
				return std::async(f, lastf.get()).get();
			});
		}

		Task(std::function<R(Args...)>&& f) :m_fn(std::move(f)){}
		Task(std::function<R(Args...)>& f) :m_fn(f){}

		~Task()
		{
		}

		void Wait()
		{
			std::async(m_fn).wait();
		}

		template<typename... Args>
		R Get(Args&&... args)
		{
			return std::async(m_fn, std::forward<Args>(args)...).get();
		}

		std::shared_future<R> Run()
		{
			return std::async(m_fn);
		}
	};
}


