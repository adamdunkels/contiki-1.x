
;---------------------------------------------------------------------       
	.export		_cs8900a_rxtxreg
	.export		_cs8900a_txcmd
	.export		_cs8900a_txlen
	.export		_cs8900a_packetpp
	.export		_cs8900a_ppdata
	

;---------------------------------------------------------------------

	_cs8900a_rxtxreg	= $c080
	_cs8900a_txcmd		= $c084
	_cs8900a_txlen		= $c086
	_cs8900a_packetpp	= $c08a
	_cs8900a_ppdata		= $c08c
