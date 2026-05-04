// arbitrary_bitset.h
// This file is part of refl-cpp
//
// Copyright (C)  2026 - Giacomo Bergami
//
// GeneralFramework is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  GeneralFramework is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with GeneralFramework. If not, see <http://www.gnu.org/licenses/>.

//
// Created by Giacomo Bergami on 04/05/2026.
//

#ifndef MORE_THAN_BASIC_ARBITRARY_BITSET_H
#define MORE_THAN_BASIC_ARBITRARY_BITSET_H
#include <cstdint>
#include <limits>
#include <string>

template<uint64_t Size>
struct arbitrary_bitset {
    uint64_t* bitset;
    constexpr const static uint64_t MAX_ARRAY_SIZE = Size/(sizeof(uint64_t)*8) + ((Size % (sizeof(uint64_t)*8)) ? 1 : 0);

    arbitrary_bitset(uint64_t* buffer = nullptr) : bitset(buffer) {}

    arbitrary_bitset<Size>& operator>>=(uint64_t __position)
{
        _M_do_right_shift(__position);
    return *this;
}

    void _M_do_right_shift(uint64_t shift)
{
        if (shift == 0)
            return;
        else if ((shift >= Size)) {
            clear();
            return;
        }

// xxxxx|xxxxx|xxxxx|xxxxx
        const uint64_t offset = shift % (sizeof(uint64_t)*8);
        uint64_t wshift = shift / (sizeof(uint64_t)*8);

        if (offset == 0) {
            for (uint64_t idx = 0; idx <MAX_ARRAY_SIZE; idx++) {
                if (idx+wshift <MAX_ARRAY_SIZE)
                    bitset[idx] = bitset[idx+wshift];
                else
                    bitset[idx] = 0;
            }
        } else {
            const uint64_t __sub_offset = ((sizeof(uint64_t)*8) - offset);
            for (uint64_t idx = 0; idx <MAX_ARRAY_SIZE; idx++) {
                if (idx+wshift <MAX_ARRAY_SIZE) {
                    const auto val_low = bitset[idx+wshift] >> offset;
                    const auto val_high = ((idx+wshift+1 <MAX_ARRAY_SIZE) ? bitset[idx+wshift+1]<<__sub_offset : 0);
                    bitset[idx] = val_low | val_high;
                }
                else
                    bitset[idx] = 0;
            }
        }

}


    void clear() {
        uint64_t N = Size/sizeof(uint64_t) + (Size % sizeof(uint64_t) ? 1 : 0);
        for (uint64_t i = 0; i < N; i++) {
            bitset[i] = 0;
        }
    }

    void fill() {
        uint64_t N = Size/sizeof(uint64_t) + (Size % sizeof(uint64_t) ? 1 : 0);
        for (uint64_t i = 0; i < N; i++) {
            bitset[i] = std::numeric_limits<uint64_t>::max();
        }
    }


   uint64_t do_find_first(uint64_t not_found = Size) const
  {
      for (uint64_t i = 0; i < Size; i++)
      {
          const uint64_t& dis = bitset[i];
          if (dis != static_cast<uint64_t>(0))
              return (i * (sizeof(uint64_t)*8)
                  + __builtin_ctzl(dis));
      }
      // not found, so return an indication of failure.
      return not_found;
  }

    arbitrary_bitset<Size>& operator &=(const arbitrary_bitset<Size>& __x)
{
        constexpr uint64_t final = Size/(sizeof(uint64_t)*8) + ((Size%(sizeof(uint64_t)*8) == 0) ? 0 : 1);
        for (uint64_t __i = 0; __i < final; __i++)
            bitset[__i] &= __x.bitset[__i];
        return *this;
}

    arbitrary_bitset<Size>& operator |=(const arbitrary_bitset<Size>& __x)
    {
        constexpr uint64_t final = Size/(sizeof(uint64_t)*8) + ((Size%(sizeof(uint64_t)*8) == 0) ? 0 : 1);
        for (uint64_t __i = 0; __i < final; __i++)
            bitset[__i] |= __x.bitset[__i];
        return *this;
    }

std::string toString() const {
        std::string s;
        s.assign(Size, '0');
        size_t n = do_find_first(Size);
        while (n < Size)
        {
            s[Size - n - 1] = '1';
            n = do_find_next(n, Size);
        }
        return s;
    }

    uint64_t do_find_next(size_t prev, size_t not_found) const
    {
        // make bound inclusive
        ++prev;

        // check out of bounds
        if (prev >= Size * (sizeof(uint64_t)*8))
            return not_found;

        // search first word
        size_t i = prev / (sizeof(uint64_t)*8);
        uint64_t thisword = bitset[i];

        // mask off bits below bound
        thisword &= (~static_cast<uint64_t>(0)) << prev % (sizeof(uint64_t)*8);

        if (thisword != static_cast<uint64_t>(0))
            return (i * (sizeof(uint64_t)*8)
                + __builtin_ctzl(thisword));

        // check subsequent words
        i++;
        for (; i < Size; i++)
        {
            thisword = bitset[i];
            if (thisword != static_cast<uint64_t>(0))
                return (i * (sizeof(uint64_t)*8)
                    + __builtin_ctzl(thisword));
        }
        // not found, so return an indication of failure.
        return not_found;
    } // end _M_do_find_next

    void invert() {
        uint64_t N = Size/sizeof(uint64_t) + (Size % sizeof(uint64_t) ? 1 : 0);
        for (uint64_t i = 0; i < N; i++) {
            bitset[i] = ~bitset[i];
        }
    }

    void set_mask(uint64_t mask, uint64_t lowbit) {
        int index = lowbit / (sizeof(uint64_t)*8);
        int offset = lowbit % (sizeof(uint64_t)*8);
        bitset[index] |= (mask<<offset);
        if (index*((sizeof(uint64_t)*8))+1 < Size) {
            mask >>= ((sizeof(uint64_t) - offset));
            bitset[index+1] |= (mask);
        }
    }
};

#endif //MORE_THAN_BASIC_ARBITRARY_BITSET_H
