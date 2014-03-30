#pragma once

//I got this code off the net at some point: no idea who the credit should go to

#include <vector>
#include "akjIVec.hpp"

namespace akj{
class BinPacker
{
public:

    // The input and output are in terms of vectors of ints to avoid
    // dependencies (although I suppose a public member struct could have been
    // used). The parameters are:
    
    // rects : An array containing the width and height of each input rect in
    // sequence, i.e. [w0][h0][w1][h1][w2][h2]... The IDs for the rects are
    // derived from the order in which they appear in the array.
    
    // packs : After packing, the outer array contains the packs (therefore
    // the number of packs is packs.size()). Each inner array contains a
    // sequence of sets of 4 ints. Each set represents a rectangle in the
    // pack. The elements in the set are 1) the rect ID, 2) the x position
    // of the rect with respect to the pack, 3) the y position of the rect
    // with respect to the pack, and 4) whether the rect was rotated (1) or
    // not (0). The widths and heights of the rects are not included, as it's
    // assumed they are stored on the caller's side (they were after all the
    // input to the function).
    
    // allowRotation : when true (the default value), the packer is allowed
    // the option of rotating the rects in the process of trying to fit them
    // into the current working area.
		struct PackResult
		{
			int mID;
			uint32_t left;
			uint32_t top;
		};

		template <class tInRect>
		bool PackSingle( const tInRect &rects,
										std::vector<PackResult>& pack,
										int pack_size)
		{
			Clear();
			m_packSize = pack_size;
			for (int i = 0; i < static_cast<int>(rects.size()); i++) {
				if (rects[i].width > m_packSize || rects[i].height > m_packSize) {
					assert(!"All rect dimensions must be <= the pack size");
				}
				m_rects.push_back(Rect(0, 0, rects[i].width, rects[i].height, i));
			}
			return PackSingle(pack);
		}

private:
		bool PackSingle( std::vector<PackResult>& pack);
    struct Rect
    {
        Rect(int size)
            : x(0), y(0), w(size), h(size), ID(-1), packed(false)
        {
            children[0] = -1;
            children[1] = -1;
        }

        Rect(int x, int y, int w, int h, int ID = 1)
            : x(x), y(y), w(w), h(h), ID(ID), packed(false)
        {
            children[0] = -1;
            children[1] = -1;
        }
        
        int GetArea() const {
            return w * h;
        }
        
        bool operator<(const Rect& rect) const {
            return GetArea() < rect.GetArea();
        }

        int  x;
        int  y;
        int  w;
        int  h;
        int  ID;
        int  children[2];
        bool packed;
    };

    void Clear();
    void Fill(int pack);
    void Split(int pack, int rect);
    bool Fits(Rect& rect1, const Rect& rect2);
    void AddPackToArray(int pack, std::vector<PackResult>& array) const;
    
    bool RectIsValid(int i) const;
    bool PackIsValid(int i) const;
    
    int               m_packSize;
    int               m_numPacked;
    std::vector<Rect> m_rects;
    std::vector<Rect> m_packs;
    std::vector<int>  m_roots;
};
}
