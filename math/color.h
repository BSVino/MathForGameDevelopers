/*
Copyright (c) 2012, Lunar Workshop, Inc.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
3. All advertising materials mentioning features or use of this software must display the following acknowledgement:
   This product includes software developed by Lunar Workshop, Inc.
4. Neither the name of the Lunar Workshop nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY LUNAR WORKSHOP INC ''AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL LUNAR WORKSHOP BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#pragma once

class Vector;

class Color
{
public:
					Color();
					Color(Vector v);
					Color(int _r, int _g, int _b);
					Color(int _r, int _g, int _b, int _a);
					Color(float _r, float _g, float _b);
					Color(float _r, float _g, float _b, float _a);

	void			SetColor(int _r, int _g, int _b, int _a);
	void			SetColor(float _r, float _g, float _b, float _a);
	void			SetRed(int _r);
	void			SetGreen(int _g);
	void			SetBlue(int _b);
	void			SetAlpha(int _a);
	void			SetAlpha(float f);

	void			SetHSL(float h, float s, float l);
	void			GetHSL(float& h, float& s, float& l);

	int				r() const { return red; };
	int				g() const { return green; };
	int				b() const { return blue; };
	int				a() const { return alpha; };

	Color	operator-(void) const;

	Color	operator+(const Color& v) const;
	Color	operator-(const Color& v) const;
	Color	operator*(float s) const;
	Color	operator/(float s) const;

	void	operator+=(const Color &v);
	void	operator-=(const Color &v);
	void	operator*=(float s);
	void	operator/=(float s);

	Color	operator*(const Color& v) const;

	friend Color operator*( float f, const Color& v )
	{
		return Color( v.red*f, v.green*f, v.blue*f, v.alpha*f );
	}

	friend Color operator/( float f, const Color& v )
	{
		return Color( f/v.red, f/v.green, f/v.blue, f/v.alpha );
	}

	operator unsigned char*()
	{
		return(&red);
	}

	operator const unsigned char*() const
	{
		return(&red);
	}

private:
	unsigned char	red;
	unsigned char	green;
	unsigned char	blue;
	unsigned char	alpha;
};
