#pragma once
class CCommandHandler
{
public:
	CCommandHandler() = delete;
	CCommandHandler(std::istream & strm);
	void Exec();
	~CCommandHandler();
private:
	std::istream & m_strm;
};

