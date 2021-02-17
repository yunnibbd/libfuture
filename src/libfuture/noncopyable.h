#ifndef __NON_COPYABLE_H__
#define __NON_COPYABLE_H__

class noncopyable
{
protected:
	noncopyable() {}
	virtual ~noncopyable() {}

private:
	noncopyable(const noncopyable&) = delete;
	void operator=(const noncopyable&) = delete;
};

#endif
