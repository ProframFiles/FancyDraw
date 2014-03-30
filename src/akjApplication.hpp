#pragma once
namespace akj{

	class cApplication
	{
	public:
		cApplication(){}
		virtual ~cApplication(){};
		virtual void Run() = 0;
		virtual void HandleEvents() = 0;

	private:

	};
}