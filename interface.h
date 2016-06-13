#ifndef BCM2DUMP_INTERFACE_H
#define BCM2DUMP_INTERFACE_H
#include <functional>
#include <memory>
#include <string>
#include <map>
#include "profile.h"
#include "util.h"
#include "io.h"

namespace bcm2dump {

class interface
{
	friend class interface_rw_base;
	typedef bcm2dump::profile profile_type;

	public:
	typedef std::shared_ptr<interface> sp;

	virtual ~interface() {}

	virtual std::string name() const = 0;
	virtual void runcmd(const std::string& cmd) = 0;
	virtual bool runcmd(const std::string& cmd, const std::string& expect, bool stop_on_match = false);

	virtual bool is_active() = 0;

	virtual void set_profile(const profile::sp& profile)
	{ m_profile = profile; }

	virtual profile_type::sp profile() const
	{ return m_profile; }

	bool is_active(const std::shared_ptr<io>& io)
	{
		m_io = io;
		if (!is_active()) {
			m_io.reset();
			return false;
		}

		return true;
	}

	void writeln(const std::string& str = "")
	{ m_io->writeln(str); }

	void write(const std::string& str)
	{ m_io->write(str); }

	std::string readln(unsigned timeout = 100) const
	{ return m_io->readln(timeout); }

	bool pending(unsigned timeout = 100) const
	{ return m_io->pending(); }

	static std::shared_ptr<interface> detect(const std::shared_ptr<io>& io);

	virtual bcm2_interface id() const = 0;

	protected:
	std::shared_ptr<io> m_io;
	profile::sp m_profile;
};

class interface_rw_base
{
	public:
	typedef std::function<void(uint32_t)> progress_listener;
	typedef std::map<std::string, std::string> args;

	virtual ~interface_rw_base()
	{ do_cleanup(); }

	virtual void set_progress_listener(const progress_listener& listener = progress_listener())
	{ m_listener = listener; }

	virtual void set_partition(const std::string& partition)
	{ m_args["partition"] = partition; }

	virtual void set_interface(const interface::sp& intf)
	{ m_intf = intf; }

	virtual void set_args(const args& args)
	{ m_args = args; }

	protected:
	virtual void init(uint32_t offset, uint32_t length) {}
	virtual void cleanup() {}

	virtual void update_progress(uint32_t offset)
	{
		if (m_listener) {
			m_listener(offset);
		}
	}

	void do_cleanup()
	{
		if (m_inited) {
			cleanup();
			m_inited = false;
		}
	}

	void do_init(uint32_t offset, uint32_t length)
	{
		do_cleanup();
		init(offset, length);
		m_inited = true;
	}

	template<class T> T arg(const std::string& name)
	{
		return lexical_cast<T>(m_args[name]);
	}

	std::string arg(const std::string& name)
	{
		return m_args[name];
	}

	progress_listener m_listener;
	interface::sp m_intf;
	const bcm2_addrspace* m_space = nullptr;
	bool m_inited = false;
	args m_args;
};

}

#endif
