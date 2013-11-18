//###########################################################################
// This file is part of gldisplay, a submodule of LImA project the
// Library for Image Acquisition
//
// Copyright (C) : 2009-2011
// European Synchrotron Radiation Facility
// BP 220, Grenoble 38043
// FRANCE
//
// This is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//###########################################################################
#ifndef __PREC_TIME_H
#define __PREC_TIME_H

#include <sys/time.h>

/********************************************************************
 * PrecTime
 ********************************************************************/

class PrecTime 
{
public:
	enum InitType {
		Invalid,
		Now,
	};

	enum ResetType{
		Unchange,
		Reset
	};

	PrecTime(InitType init = Invalid)
	{ 
		if (init == Now)
			setNow();
		else
			setInvalid(); 
	}

	void setInvalid()
	{ t.tv_sec = t.tv_usec = 0; }

	bool isValid() const
	{ return t.tv_sec != 0; }

	void setNow()
	{ gettimeofday(&t, NULL); }

        double operator -(const PrecTime &t0) const
	{ return ((t.tv_sec - t0.t.tv_sec) + 
		  (t.tv_usec - t0.t.tv_usec) * 1e-6); }

	static PrecTime now()
	{ return PrecTime(Now); }

	double timeElapsed(ResetType reset = Unchange)
	{ 
		PrecTime now(Now);
		double sec = now - *this;
		if (reset == Reset)
			*this = now;
		return sec;
	}

private:
	struct timeval t;
};

/********************************************************************
 * Rate
 ********************************************************************/

class Rate
{
public:
	Rate()
	{ Init(); }
	Rate(const Rate &o)
	{ Init(o.rate, o.update_rate); }
	Rate(float r)
	{ Init(r); }
	Rate(Rate *upd_rate)
	{ Init(1.0, upd_rate); }

	float get() const
	{ return rate; }

	void update()
	{
		count++;
		if (update_rate && !update_rate->isTime(this))
			return;

		rate = double(count) / last.timeElapsed(PrecTime::Reset);
		count = 0;
	}

	void set(float r)
	{
		rate = r;
		count = 0;
		last.setNow();
	}

	void setUpdateRate(Rate *upd_rate)
	{ update_rate = upd_rate; }

	float period() const
	{ return rate ? (1.0 / rate) : 0; }

	float remainingTime(Rate *other = NULL)
	{ 
		if (other == NULL)
			other = this;

		return period() - other->last.timeElapsed();
	}

	bool isTime(Rate *other = NULL)
	{ 
		if (remainingTime(other) > 0)
			return false;

		if (!other || (other == this))
			last.setNow();
		return true;
	}

	Rate &operator =(float r)
	{
		set(r);
		return *this;
	}

protected:
	void Init(float r = 1.0, Rate *upd_rate = NULL)
	{
		set(r);
		setUpdateRate(upd_rate);
	}
			

private:
	float rate;
	int count;
	PrecTime last;
	Rate *update_rate;
};


#endif /* __PREC_TIME_H */
