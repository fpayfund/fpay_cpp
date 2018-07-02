#include "core/sox/selector.h"
#include "snox.h"

namespace sox {

Selector::~Selector() {
	if (m_interrupt_handler)
		m_interrupt_handler->destroy();
}

void Selector::set_interrupt_handler(Handler * handler) {
	if (m_interrupt_handler)
		m_interrupt_handler->destroy();
	m_interrupt_handler = handler;
}

void Selector::notify_event(Handler * s, int event) {
	//	SYS_LOG(Debug, <<"handle= "<<s<<" "<<event);
	// assert(s);
	if (isRemoved(s)) {
		log(Info, "DESTROY IN LOOP FOUND");
		return;
	}
	// ���浱ǰ�������
	env::s_CurrentHandler = s;
	try {s->handle(event);}
	catch (std::exception & ex)
	{
		log(Error, "Sox Exception:%s, %s", ex.what(), s->toString().data());
		//s->destroy();
	}
	// �����ǰ�����
	env::s_CurrentHandler = NULL; // ���Ǻܱ�Ҫ�����մ�ʩ
}

} // namespace sox
