#pragma once
#include <algorithm>
#include <future>
using namespace std;
#include "TaskGroup.hpp"

namespace Cosmos
{
	template <class Iterator, class Function>
	void ParallelForeach(Iterator& begin, Iterator& end, Function& func)
	{
		auto partNum = std::thread::hardware_concurrency();
		auto blockSize = std::distance(begin, end) / partNum;
		Iterator last = begin;
		if (blockSize > 0)
		{
			std::advance(last, (partNum - 1) * blockSize);
		}
		else
		{
			last = end;
			blockSize = 1;
		}

		std::vector<std::future<void>> futures;
		// first p - 1 groups
		for (; begin != last; std::advance(begin, blockSize))
		{
			futures.emplace_back(std::async([begin, blockSize, &func]
			{
				std::for_each(begin, begin + blockSize, func);
			}));
		}

		//// last group
		futures.emplace_back(std::async([&begin, &end, &func]{std::for_each(begin, end, func); }));

		std::for_each(futures.begin(), futures.end(), [](std::future<void>& futuer)
		{
			futuer.get();
		});
	}

	template <typename Range, typename ReduceFunc>
	inline typename Range::value_type ParallelReduce(Range& range,
		typename Range::value_type &init, ReduceFunc reduceFunc)
	{
		return ParallelReduce<Range, ReduceFunc>(range, init, reduceFunc, reduceFunc);
	}

	template <typename Range, typename RangeFunc, typename ReduceFunc>
	inline typename Range::value_type ParallelReduce(Range& range,
		typename Range::value_type &init, RangeFunc& rangeFunc, ReduceFunc& reduceFunc)
	{
		auto partNum = std::thread::hardware_concurrency();
		auto begin = std::begin(range); auto end = std::end(range);
		auto blockSize = std::distance(begin, end) / partNum;
		typename Range::iterator last = begin;
		if (blockSize > 0)
		{
			std::advance(last, (partNum - 1) * blockSize);
		}
		else
		{
			last = end;
			blockSize = 1;
		}

		typedef typename Range::value_type ValueType;
		std::vector<std::future<ValueType>> futures;
		// first p - 1 groups
		for (; begin != last; std::advance(begin, blockSize))
		{
			futures.emplace_back(std::async([begin, &init, blockSize, &rangeFunc]
			{
				return rangeFunc(begin, begin + blockSize, init);
			}));
		}

		//// last group
		futures.emplace_back(std::async([&begin, &end, &init, &rangeFunc]{return rangeFunc(begin, end, init); }));

		vector<ValueType> results;
		std::for_each(futures.begin(), futures.end(), [&results](std::future<ValueType>& futuer)
		{
			results.emplace_back(futuer.get());
		});

		return reduceFunc(results.begin(), results.end(), init);
	}

	template<typename... Funs>
	void ParallelInvoke(Funs&&... rest)
	{
		TaskGroup group;
		group.Run(std::forward<Funs>(rest)...);
		group.Wait();
	}
}


