;
; Forced importing code for Contiki (Apple2 version)
;
; These symbols are necessary for (at least some) apps, but are
; initialized by constructors which are currently not supported
; for apps. Therefore we include these symbols into the Contiki
; kernel to make them thus available to the apps indirectly via
; the contiki-labels mechanism.

	.forceimport	__dos_type
