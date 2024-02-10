#include "iohub.hpp"

Iohub::Iohub()
{

}

Iohub::~Iohub()
{

}

int Iohub::register_port(unsigned long port, Idevice *i_device)
{
	return 0;
}

void Iohub::unregister_port(unsigned long port)
{

}

int Iohub::register_mmio(unsigned long mmio, Idevice *i_device)
{
	return 0;
}

void Iohub::unregister_mmio(unsigned long mmio)
{

}

int Iohub::dispatch_port_access(unsigned long type,
				unsigned long size,
				unsigned long &val)
{
	return 0;
}

int Iohub::dispatch_mmio_access(unsigned long type,
				unsigned long size,
				unsigned long &val)
{
	return 0;
}

int Iohub::do_register(io_map &map, unsigned long index, Idevice *i_device)
{
	return 0;
}

void Iohub::do_unregister(io_map &map, unsigned long index)
{

}
