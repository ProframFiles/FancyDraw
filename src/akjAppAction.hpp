#pragma once
#include "akjApplicationObject.hpp"
#include "akjInput.hpp"

namespace akj
{
class cAppAction : public cApplicationObject
{
public:
	cAppAction(cWrappingCounter& counter,
							const Twine& action_name,
							std::function<void(void)>&& func)
		: cApplicationObject(counter)
		, mFunction(func)
		, mActionName(action_name.str())
	{}
	~cAppAction(){};

	void Execute()
	{
		if(!mFunction)
		{
			AKJ_THROW(Twine(mActionName) + " does not have a valid action");
		}
		mFunction();
	}

private:
	cInputID mBinding;
	std::string mActionName;
	std::function<void(void)> mFunction;
};

}
