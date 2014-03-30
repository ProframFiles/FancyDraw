#pragma once

namespace akj
{
	enum eHilbertDirection
	{
		HILBERT_LEFT,
		HILBERT_RIGHT,
		HILBERT_UP,
		HILBERT_DOWN
	};
  template <class tHilbertAction>
  class HilbertTransform {
  public:
    HilbertTransform(tHilbertAction& action)
    : mAction(action)
		, mX(0), mY(0), mTopLevel(256)
		{}

		void TraverseHilbert(int level, int x_start, int y_start,
			eHilbertDirection direction = HILBERT_LEFT, uint32_t level_skip = 0)
    {
			mTopLevel = level-level_skip;
      mX = x_start;
      mY = y_start;
			mAction.MoveTo(mX, mY);
      TraverseHilbert(level, direction);
    }

    private:
			void TraverseHilbert(int level, eHilbertDirection direction)
    {
      if (level == 1)
      {
        switch (direction) {
				case HILBERT_LEFT:
          mAction.MoveTo(++mX, mY);
          mAction.MoveTo(mX, ++mY);
          mAction.MoveTo(--mX, mY);
          break;
				case HILBERT_RIGHT:
          mAction.MoveTo(--mX, mY);
          mAction.MoveTo(mX, --mY);
          mAction.MoveTo(++mX, mY);
          break;
				case HILBERT_UP:
          mAction.MoveTo(mX, ++mY);
          mAction.MoveTo(++mX, mY);
          mAction.MoveTo(mX, --mY);
          break;
				case HILBERT_DOWN:
          mAction.MoveTo(mX, --mY);
          mAction.MoveTo(--mX, mY);
          mAction.MoveTo(mX, ++mY);
          break;
        }
      }
      else {
        switch (direction) 
        {
				case HILBERT_LEFT:
					TraverseHilbert(level - 1, HILBERT_UP);
          mAction.MoveTo(++mX, mY);
					TraverseHilbert(level - 1, HILBERT_LEFT);
          mAction.MoveTo(mX, ++mY);
					TraverseHilbert(level - 1, HILBERT_LEFT);
          mAction.MoveTo(--mX, mY);
					TraverseHilbert(level - 1, HILBERT_DOWN);
          break;
				case HILBERT_RIGHT:
					TraverseHilbert(level - 1, HILBERT_DOWN);
          mAction.MoveTo(--mX, mY);
					TraverseHilbert(level - 1, HILBERT_RIGHT);
          mAction.MoveTo(mX, --mY);
					TraverseHilbert(level - 1, HILBERT_RIGHT);
          mAction.MoveTo(++mX, mY);
					TraverseHilbert(level - 1, HILBERT_UP);
          break;
				case HILBERT_UP:
					TraverseHilbert(level - 1, HILBERT_LEFT);
          mAction.MoveTo(mX, ++mY);
					TraverseHilbert(level - 1, HILBERT_UP);
          mAction.MoveTo(++mX, mY);
					TraverseHilbert(level - 1, HILBERT_UP);
          mAction.MoveTo(mX, --mY);
					TraverseHilbert(level - 1, HILBERT_RIGHT);
          break;
				case HILBERT_DOWN:
					TraverseHilbert(level - 1, HILBERT_RIGHT);
          mAction.MoveTo(mX, --mY);
					TraverseHilbert(level - 1, HILBERT_DOWN);
          mAction.MoveTo(--mX, mY);
					TraverseHilbert(level - 1, HILBERT_DOWN);
          mAction.MoveTo(mX, ++mY);
					TraverseHilbert(level - 1, HILBERT_LEFT);
          break;
        } /* switch */
      } /* if */
    }

    int mX;
    int mY;
		uint32_t mTopLevel;
		uint32_t mBottomLevel;
    tHilbertAction& mAction;
   
  };

}
