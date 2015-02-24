#pragma once
#include "Task.hpp"

namespace Cosmos
{
	template <typename Range>
	Task<vector<typename Range::value_type::return_type>()> WhenAll(Range& range)
	{
		typedef typename Range::value_type::return_type ReturnType;
		auto task = [&range]
		{
			vector<std::shared_future<ReturnType>> fv;
			for (auto & task : range)
			{
				fv.emplace_back(task.Run());
			}

			vector<ReturnType> v;
			for (auto& item : fv)
			{
				v.emplace_back(item.get());
			}

			return v;
		};

		return task;
	}

	template <typename Range>
	Task<std::pair<int, typename Range::value_type::return_type>()> WhenAny(Range& range)
	{
		auto task = [&range]
		{
			using namespace Detail;
			return GetAnyResultPair(TransForm(range));
		};

		return task;
	}

	namespace Detail
	{
		template <typename R>
		struct RangeTrait
		{
			typedef R Type;
		};

		template <typename R>
		struct RangeTrait<std::shared_future<R>>
		{
			typedef R Type;
		};

		template <typename Range>
		vector<std::shared_future<typename Range::value_type::return_type>> TransForm(Range& range)
		{
			typedef typename Range::value_type::return_type ReturnType;
			vector<std::shared_future<ReturnType>> fv;
			for (auto & task : range)
			{
				fv.emplace_back(task.Run());
			}

			return fv;
		}

		template<typename Range>
		std::pair<int, typename RangeTrait<typename Range::value_type>::Type> GetAnyResultPair(Range& fv)
		{
			size_t size = fv.size();
			for (;;)
			{
				for (size_t i = 0; i < size; i++)
				{
					if (fv[i].wait_for(std::chrono::milliseconds(1)) == std::future_status::ready)
					{
						return std::make_pair(i, fv[i].get());
					}
				}
			}
		}
	}
}


