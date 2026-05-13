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
#include <set>
#include <string>
#include <unordered_set>

#include "jackbergus/data_structures/IntervalTree.h"


struct arbitrary_bitset {

    uint64_t Size;
    uint64_t MAX_ARRAY_SIZE;
    using T = unsigned char;
    constexpr static const uint64_t BYTE_SIZE = sizeof(T);
    constexpr static const uint64_t BIT_SIZE = BYTE_SIZE * 8;
    T* bitset;

    arbitrary_bitset(T* buffer = nullptr, uint64_t Size = 0) : bitset(buffer), Size(Size), MAX_ARRAY_SIZE(Size/(BIT_SIZE) + ((Size % (BIT_SIZE)) ? 1 : 0)) {}
    arbitrary_bitset& operator>>=(uint64_t __position)
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
        const uint64_t offset = shift % (BIT_SIZE);
        uint64_t wshift = shift / (BIT_SIZE);

        if (offset == 0) {
            for (uint64_t idx = 0; idx <MAX_ARRAY_SIZE; idx++) {
                if (idx+wshift <MAX_ARRAY_SIZE)
                    bitset[idx] = bitset[idx+wshift];
                else
                    bitset[idx] = 0;
            }
        } else {
            const uint64_t __sub_offset = ((BIT_SIZE) - offset);
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
        uint64_t N = Size/BYTE_SIZE + (Size % BYTE_SIZE ? 1 : 0);
        for (uint64_t i = 0; i < N; i++) {
            bitset[i] = 0;
        }
    }

    void fill() {
        uint64_t N = Size/BYTE_SIZE + (Size % BYTE_SIZE ? 1 : 0);
        for (uint64_t i = 0; i < N; i++) {
            bitset[i] = std::numeric_limits<unsigned char>::max();
        }
    }


   uint64_t do_find_first(uint64_t not_found) const
  {
      for (uint64_t i = 0; i < Size; i++)
      {
          const uint64_t& dis = bitset[i];
          if (dis != static_cast<uint64_t>(0))
              return (i * (BIT_SIZE)
                  + __builtin_ctzl(dis));
      }
      // not found, so return an indication of failure.
      return not_found;
  }

    arbitrary_bitset& operator &=(const arbitrary_bitset& __x)
{
        uint64_t final = Size/(BIT_SIZE) + ((Size%(BIT_SIZE) == 0) ? 0 : 1);
        for (uint64_t __i = 0; __i < final; __i++)
            bitset[__i] &= __x.bitset[__i];
        return *this;
}

    arbitrary_bitset& operator |=(const arbitrary_bitset& __x)
    {
        uint64_t final = Size/(BIT_SIZE) + ((Size%(BIT_SIZE) == 0) ? 0 : 1);
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

    std::set<uint64_t> deltaFromIntervalTreeSlot(const IntervalTree<uint64_t, uint64_t>& it, const arbitrary_bitset& rhs) {
        std::set<uint64_t> result;
        uint64_t final = Size/(BIT_SIZE) + ((Size%(BIT_SIZE) == 0) ? 0 : 1);
        // static_assert(final == MAX_ARRAY_SIZE);
        uint64_t tmp_bitset[MAX_ARRAY_SIZE];
        for (uint64_t __i = 0; __i < final; __i++) {
            tmp_bitset[__i] = bitset[__i] ^ rhs.bitset[__i];
        }


        uint64_t current_bit = Size;
        for (uint64_t i = 0; i < Size; i++)
        {
            const uint64_t& dis = tmp_bitset[i];
            if (dis != static_cast<uint64_t>(0)) {
                current_bit = (i * (BIT_SIZE)
                    + __builtin_ctzl(dis));
                break;
            }
        }
        Interval<uint64_t, uint64_t> tmp{0, 0, 0};
        while (current_bit < Size)
        {
            tmp.low = current_bit;
            tmp.high = current_bit;
            auto result_ptr = it.lookup(tmp);
            if (!result_ptr) {
                std::cout << "ERROR" << std::endl;
                current_bit = Size;
            } else {
                result.insert(result_ptr->value);
                ++current_bit;


                // check out of bounds
                if (current_bit >= Size /** (BIT_SIZE)*/)
                    break;

                // search first word
                size_t i = current_bit / (BIT_SIZE);
                uint64_t thisword = tmp_bitset[i];

                // mask off bits below bound
                thisword &= (~static_cast<uint64_t>(0)) << current_bit % (BIT_SIZE);

                if (thisword != static_cast<uint64_t>(0))
                {
                    current_bit = (i * (BIT_SIZE) + __builtin_ctzl(thisword));
                } else {
                    // check subsequent words
                    i++;
                    bool found = false;
                    for (; i < Size; i++)
                    {
                        thisword = tmp_bitset[i];
                        if (thisword != static_cast<uint64_t>(0)) {
                            current_bit = (i * (BIT_SIZE)
                                + __builtin_ctzl(thisword));
                            found = true;
                            break;
                        }
                    }
                    if (!found) {
                        current_bit = Size;
                    }
                }
            }
        }

        // not found, so return an indication of failure.
        return result;
    }

    uint64_t do_find_next(size_t prev, size_t not_found) const
    {
        // make bound inclusive
        ++prev;

        // check out of bounds
        if (prev >= Size /** (BIT_SIZE)*/)
            return not_found;

        // search first word
        size_t i = prev / (BIT_SIZE);
        uint64_t thisword = bitset[i];

        // mask off bits below bound
        thisword &= (~static_cast<uint64_t>(0)) << prev % (BIT_SIZE);

        if (thisword != static_cast<uint64_t>(0))
            return (i * (BIT_SIZE)
                + __builtin_ctzl(thisword));

        // check subsequent words
        i++;
        for (; i < Size; i++)
        {
            thisword = bitset[i];
            if (thisword != static_cast<uint64_t>(0))
                return (i * (BIT_SIZE)
                    + __builtin_ctzl(thisword));
        }
        // not found, so return an indication of failure.
        return not_found;
    } // end _M_do_find_next

    void invert() {
        uint64_t N = Size/BYTE_SIZE + (Size % BYTE_SIZE ? 1 : 0);
        for (uint64_t i = 0; i < N; i++) {
            bitset[i] = ~bitset[i];
        }
    }

    void set_mask(uint64_t mask, uint64_t lowbit) {
        uint64_t index = lowbit / BIT_SIZE;
        uint64_t offset = lowbit % BIT_SIZE;
        auto val = (mask<<offset);
        for (uint64_t j = 0; index+j< MAX_ARRAY_SIZE && j<sizeof(uint64_t)/BYTE_SIZE; j++) {
            bitset[index+j] |= ((unsigned char*)&val)[j];
        }
        mask >>= ((sizeof(uint64_t)*8 - offset));
        if (mask != 0) {
            for (uint64_t j = 0; index+j+sizeof(uint64_t)/BYTE_SIZE< MAX_ARRAY_SIZE && j<sizeof(uint64_t)/BYTE_SIZE; j++) {
                bitset[index+j+sizeof(uint64_t)/BYTE_SIZE] |= ((unsigned char*)&mask)[j];
            }
        }
        // bitset[index] |= (mask<<offset);
        // if (index*((sizeof(uint64_t)))+1 < Size) {
        //     mask >>= ((sizeof(uint64_t)*8 - offset));
        //     bitset[index+1] |= (mask);
        // }
    }
};

#endif //MORE_THAN_BASIC_ARBITRARY_BITSET_H
