#include "BinPacker.hpp"
#include <cassert>
#include <algorithm>
namespace akj{
// ---------------------------------------------------------------------------
bool BinPacker::PackSingle(std::vector<PackResult>& pack)
{

    // Sort from greatest to least area
    std::sort(m_rects.rbegin(), m_rects.rend());

    // Pack

    m_packs.push_back(Rect(m_packSize));
    m_roots.push_back(0);
    Fill(0);
    if(m_numPacked < (int)m_rects.size())
		{
			return false;
		}

		pack.clear();
		AddPackToArray(m_roots[0], pack);
    
    // Check and make sure all rects were packed
    for (size_t i = 0; i < m_rects.size(); ++i) {
        if (!m_rects[i].packed) {
            assert(!"Not all rects were packed");
        }
    }
		return true;
}
// ---------------------------------------------------------------------------
void BinPacker::Clear()
{
    m_packSize = 0;
    m_numPacked = 0;
    m_rects.clear();
    m_packs.clear();
    m_roots.clear();
}
// ---------------------------------------------------------------------------
void BinPacker::Fill(int pack)
{
    assert(PackIsValid(pack));

    int i = pack;

    // For each rect
    for (int j = 0; j < static_cast<int>(m_rects.size()); ++j) {
        // If it's not already packed
        if (!m_rects[j].packed) {
            // If it fits in the current working area
            if (Fits(m_rects[j], m_packs[i])) {
                // Store in lower-left of working area, split, and recurse
                ++m_numPacked;
                Split(i, j);
                Fill(m_packs[i].children[0]);
                Fill(m_packs[i].children[1]);
                return;
            }
        }
    }
}
// ---------------------------------------------------------------------------
void BinPacker::Split(int pack, int rect)
{
    assert(PackIsValid(pack));
    assert(RectIsValid(rect));

    int i = pack;
    int j = rect;

    // Split the working area either horizontally or vertically with respect
    // to the rect we're storing, such that we get the largest possible child
    // area.

    Rect left = m_packs[i];
    Rect right = m_packs[i];
    Rect bottom = m_packs[i];
    Rect top = m_packs[i];

    left.y += m_rects[j].h;
    left.w = m_rects[j].w;
    left.h -= m_rects[j].h;
    right.x += m_rects[j].w;
    right.w -= m_rects[j].w;

    bottom.x += m_rects[j].w;
    bottom.h = m_rects[j].h;
    bottom.w -= m_rects[j].w;
    top.y += m_rects[j].h;
    top.h -= m_rects[j].h;

    int maxLeftRightArea = left.GetArea();
    if (right.GetArea() > maxLeftRightArea) {
        maxLeftRightArea = right.GetArea();
    }

    int maxBottomTopArea = bottom.GetArea();
    if (top.GetArea() > maxBottomTopArea) {
        maxBottomTopArea = top.GetArea();
    }

    if (maxLeftRightArea > maxBottomTopArea) {
        if (left.GetArea() > right.GetArea()) {
            m_packs.push_back(left);
            m_packs.push_back(right);
        } else {
            m_packs.push_back(right);
            m_packs.push_back(left);
        }
    } else {
        if (bottom.GetArea() > top.GetArea()) {
            m_packs.push_back(bottom);
            m_packs.push_back(top);
        } else {
            m_packs.push_back(top);
            m_packs.push_back(bottom);
        }
    }

    // This pack area now represents the rect we've just stored, so save the
    // relevant info to it, and assign children.
    m_packs[i].w = m_rects[j].w;
    m_packs[i].h = m_rects[j].h;
    m_packs[i].ID = m_rects[j].ID;
    m_packs[i].children[0] = static_cast<int>(m_packs.size()) - 2;
    m_packs[i].children[1] = static_cast<int>(m_packs.size()) - 1;

    // Done with the rect
    m_rects[j].packed = true;
}
// ---------------------------------------------------------------------------
bool BinPacker::Fits(Rect& rect1, const Rect& rect2)
{
    // Check to see if rect1 fits in rect2, and rotate rect1 if that will
    // enable it to fit.

    if (rect1.w <= rect2.w && rect1.h <= rect2.h) {
        return true;
		} else {
        return false;
    }
}
// ---------------------------------------------------------------------------
void BinPacker::AddPackToArray(int pack, std::vector<PackResult>& array) const
{
    assert(PackIsValid(pack));

    int i = pack;
    if (m_packs[i].ID != -1) {
        array.emplace_back();
				array.back().mID = m_packs[i].ID;
        array.back().left = m_packs[i].x;
        array.back().top = m_packs[i].y;

        if (m_packs[i].children[0] != -1) {
            AddPackToArray(m_packs[i].children[0], array);
        }
        if (m_packs[i].children[1] != -1) {
            AddPackToArray(m_packs[i].children[1], array);
        }
    }
}
// ---------------------------------------------------------------------------
bool BinPacker::RectIsValid(int i) const
{
    return i >= 0 && i < (int)m_rects.size();
}
// ---------------------------------------------------------------------------
bool BinPacker::PackIsValid(int i) const
{
    return i >= 0 && i < (int)m_packs.size();
}
// ---------------------------------------------------------------------------
}