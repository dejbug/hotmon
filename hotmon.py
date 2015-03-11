import ctypes as c
import ctypes.wintypes as w


INFINITE = 0xFFFFFFFF


E_OK				= 0
E_NULL_ARG			= 1
E_OUT_OF_MEMORY		= 2
E_EVENT_CREATE		= 3
E_EVENT_USAGE		= 4
E_NOT_RUNNING		= 5
E_STILL_RUNNING		= 6
E_ALREADY_RUNNING	= 7
E_START_TIMEOUT		= 8
E_STOP_TIMEOUT		= 9
E_ATOM_CREATE		= 10
E_HOTKEY_NOT_INIT	= 11
E_TIMEOUT			= 12
E_HOTKEY_REGISTER	= 13
E_HOTKEY_UNREGISTER	= 14


E_LABELS = [
	"E_OK",
	"E_NULL_ARG",
	"E_OUT_OF_MEMORY",
	"E_EVENT_CREATE",
	"E_EVENT_USAGE",
	"E_NOT_RUNNING",
	"E_STILL_RUNNING",
	"E_ALREADY_RUNNING",
	"E_STARTUP_TIMEOUT",
	"E_ATOM_CREATE",
	"E_HOTKEY_NOT_INIT",
	"E_TIMEOUT",
	"E_HOTKEY_REGISTER",
	"E_HOTKEY_UNREGISTER",
]


class Error(Exception): pass

class HotmonError(Error):
	def __init__(self, code):
		self.code = code
		Error.__init__(self, "[%d] (%s)" % (self.code, E_LABELS[self.code]))


dll = c.CDLL("hotmon.dll")


CALLBACK = c.CFUNCTYPE(None, w.LPVOID)


class HOTMON(c.Structure):
	_fields_ = [
		("hwnd", w.HWND),
		("event", w.HANDLE),
		("tid", w.DWORD),
	]
PHOTMON = c.POINTER(HOTMON)
PPHOTMON = c.POINTER(PHOTMON)


class HOTKEY(c.Structure):
	_fields_ = [
		("id", w.ATOM),
		("vk", w.UINT),
		("mod", w.UINT),
		("callback", CALLBACK),
		("param", w.LPARAM),
	]
PHOTKEY = c.POINTER(HOTKEY)


def errcheck(result, func, args):
	if E_OK != result:
		raise HotmonError(result)
	return args
	
	
class API(object):
	def __init__(self, *signature):
		"""The arguments to this constructor are caught in 'signature',
		which is a sequence of 3-tuples (arg_name, arg_type, arg_default).
		In this version, all arguments are supposed to be input arguments
		and will have the default value of 0."""
		
		arg_types = (c.c_int, ) + tuple(item[1] for item in signature)
		
		self.proto = c.CFUNCTYPE(*arg_types)
		self.paraf = tuple((1, item[0], item[2]) for item in signature)
		
	def __call__(self, func):
		fwrap = self.proto((func.__name__, dll), self.paraf)
		fwrap.errcheck = errcheck
		return fwrap


@API(("hm", PPHOTMON, None))
def hmCreate(): pass

@API(("hm", PPHOTMON, None))
def hmDelete(): pass

@API(("hm", PPHOTMON, None), ("msec", w.DWORD, 3000))
def hmStart(): pass

@API(("hm", PPHOTMON, None), ("msec", w.DWORD, 3000))
def hmStop(): pass

@API(("hm", PPHOTMON, None), ("hk", PHOTKEY, None), ("vk", w.UINT, 0), ("mod", w.UINT, 0), ("callback", CALLBACK, None), ("param", w.LPVOID, None))
def hmCreateHotkey(): pass

@API(("hm", PPHOTMON, None), ("hk", PHOTKEY, None))
def hmAddHotkey(): pass

@API(("hm", PPHOTMON, None), ("hk", PHOTKEY, None))
def hmRegister(): pass

@API(("hm", PPHOTMON, None), ("hk", PHOTKEY, None))
def hmUnregister(): pass
