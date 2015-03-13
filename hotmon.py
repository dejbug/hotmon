import ctypes as c
import ctypes.wintypes as w
import threading

# These are all the includes we need, but i like to keep my namespaces
#	separate.
#from ctypes import CDLL, CFUNCTYPE, POINTER, Structure, c_int
#from ctypes.wintypes import LPVOID, HANDLE, HWND, ATOM, DWORD, UINT
#from threading import Event as threading_Event


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
E_TIMEOUT			= 12	# obsolete
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


class ApiError(Error):
	def __init__(self, code):
		self.code = code
		Error.__init__(self, "[%d] (%s)" % (self.code, E_LABELS[self.code]))
		
		
class HotmonError(ApiError): pass


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
		raise ApiError(result)
	return args
	
	
class API(object):
	def __init__(self, *signature):
		"""The arguments to this decorator are caught in 'signature',
		which is a sequence of 3-tuples (arg_name, arg_type, arg_default).
		In this version, all arguments are supposed to be input arguments."""
		
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


class Hotmon(object):
	def __init__(self):
		self.event = threading.Event()
		
		# we need to keep the callbacks around so they don't
		#	get garbage collected prematurely.
		self.callbacks = []
		
		self.hm = None
		
	def create(self):
		if self.hm:
			raise HotmonError, E_ALREADY_RUNNING
			
		self.hm = PHOTMON()
		hmCreate(self.hm)
		hmStart(self.hm)
		
	def destroy(self):
		if self.hm:
			self.stop()
			hmDelete(self.hm)
			self.hm = None
			
	def stop(self):
		# stop, if not already
		try:
			hmStop(self.hm)
		except ApiError, e:
			if E_NOT_RUNNING != e.code:
				raise e
		finally:
			self.event.set()
				
	def wait(self):
		if None is self.hm:
			raise HotmonError, E_NOT_RUNNING
			
		# wait for stop() to be called
		self.event.wait()
			
	def add(self, callback, vk, mod=0):
		if None is self.hm:
			raise HotmonError, E_NOT_RUNNING
			
		#TODO: add a check to see whether the vk+mod was
		#	already defined. then do something useful with
		#	that information :) (i.e. either redefine
		#	it or raise hell).
		
		cb = CALLBACK(callback)
		self.callbacks.append(cb)
		
		hk = HOTKEY()
		hmCreateHotkey(self.hm, hk, vk, mod, cb)
		hmAddHotkey(self.hm, hk)
		
