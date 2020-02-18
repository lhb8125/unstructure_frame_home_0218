/*
The MIT License

Copyright (c) 2019 Hanfeng GU <hanfenggu@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

/**
* @File: patch.hpp
* @author: Hanfeng GU
* @Email:
* @Date:   2019-10-11 10:09:18
* @Last Modified by:   Hanfeng GU
* @Last Modified time: 2019-11-08 11:26:39
*/


#ifndef PATCH_HPP
#define PATCH_HPP

#include "container.hpp"
#include "base.hpp"

namespace HSF
{

/**
 * @brief      This class describes a patch.
 */
class Patch
{
private:
	/**
	 * @param faces size.
	 */
	label size_;

	/**
	 * @param addressing.
	 */
	label* addressing_;

	/**
	 * @param patch type.
	 */
	Word setType_;

	/**
	 * @param the neighbor processor ID.
	 */
	label nbrID_;

public:
	/**
	 * @brief      Constructs a new instance.
	 */
	Patch();

	/**
	 * @brief      Constructs a new instance.
	 */
	Patch
	(
		label  size,
		label* addressing,
		label  nbrID
	);

	/**
	 * @brief      Destroys the object.
	 */
	~Patch();


	/**
	 * @brief      Gets the patch size.
	 * @return     The patch IDs's and data_'s size.
	 */
	inline label getSize(){return size_;}

	/**
	 * @brief      Gets IDs_ address.
	 * @return     IDs_ address.
	 */
	inline label* getAddressing(){return addressing_;}

	/**
	 * @brief      Gets the patch type.
	 * @return     The type.
	 */
	inline Word getType(){return setType_;}


	/**
	 * @brief      Gets the neighbor processor id.
	 * @return     The neighbor processor ID.
	 */
	inline label getNbrID(){return nbrID_;}
};

} //- end namespace HSF

#endif //- end Patch_hpp
