/*
 * Copyright 2011, Tim Branyen @tbranyen <tim@tabdeveloper.com>
 * Dual licensed under the MIT and GPL licenses.
 *
 * TODO
 *  Set cwiid_set_err so we can print helpful messages
 */

#include <v8.h>
#include <node.h>
#include <node_events.h>
#include <stdlib.h>

#include <bluetooth/bluetooth.h>
#include "../vendor/cwiid/libcwiid/cwiid.h"

#include "../include/wiimote.h"

using namespace v8;
using namespace node;

int cwiid_set_err(cwiid_err_t *err);
void WiiMote_cwiid_err(struct wiimote *wiimote, const char *str, va_list ap) {
	(void)wiimote;

	// TODO move this into a error object, so we can return it in the error objects
	vfprintf(stdout, str, ap);
	fprintf(stdout, "\n");
}


/**
 * Constructor: WiiMote
 */
WiiMote::WiiMote() {
	DEBUG_OPT("WiiMote()");
	wiimote = NULL;
};
/**
 * Deconstructor: WiiMote
 */
WiiMote::~WiiMote() {
	DEBUG_OPT("~WiiMote()");
	Disconnect();
};

#define NODE_DEFINE_CONSTANT_NAME(target, name, constant)                 \
  (target)->Set(v8::String::NewSymbol(name),                              \
                v8::Integer::New(constant),                               \
                static_cast<v8::PropertyAttribute>(v8::ReadOnly|v8::DontDelete))

void WiiMote::Initialize (Handle<v8::Object> target) {
  HandleScope scope;

  DEBUG("WiiMote::Initialize()");

  cwiid_set_err(&WiiMote_cwiid_err);

  Local<FunctionTemplate> t = FunctionTemplate::New(New);
  t->Inherit(EventEmitter::constructor_template);

  ir_event      = NODE_PSYMBOL("ir");
  acc_event     = NODE_PSYMBOL("acc");
  nunchuk_event = NODE_PSYMBOL("nunchuk");
  error_event   = NODE_PSYMBOL("error");
  button_event  = NODE_PSYMBOL("button");
  status_event  = NODE_PSYMBOL("status");

  constructor_template = Persistent<FunctionTemplate>::New(t);
  constructor_template->InstanceTemplate()->SetInternalFieldCount(1);
  constructor_template->SetClassName(String::NewSymbol("WiiMote"));

  NODE_SET_PROTOTYPE_METHOD(constructor_template, "connect", Connect);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "disconnect", Disconnect);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "rumble", Rumble);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "led", Led);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "ir", IrReporting);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "acc", AccReporting);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "ext", ExtReporting);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "button", ButtonReporting);

  NODE_DEFINE_CONSTANT_NAME(target, "IR_X_MAX", CWIID_IR_X_MAX);
  NODE_DEFINE_CONSTANT_NAME(target, "IR_Y_MAX", CWIID_IR_Y_MAX);
  NODE_DEFINE_CONSTANT_NAME(target, "IR_SRC_COUNT", CWIID_IR_SRC_COUNT);

  NODE_DEFINE_CONSTANT_NAME(target, "BATTERY_MAX", CWIID_BATTERY_MAX);

  NODE_DEFINE_CONSTANT_NAME(target, "BTN_1", CWIID_BTN_1);
  NODE_DEFINE_CONSTANT_NAME(target, "BTN_2", CWIID_BTN_2);
  NODE_DEFINE_CONSTANT_NAME(target, "BTN_A", CWIID_BTN_A);
  NODE_DEFINE_CONSTANT_NAME(target, "BTN_B", CWIID_BTN_B);
  NODE_DEFINE_CONSTANT_NAME(target, "BTN_MINUS", CWIID_BTN_MINUS);
  NODE_DEFINE_CONSTANT_NAME(target, "BTN_PLUS",  CWIID_BTN_PLUS);
  NODE_DEFINE_CONSTANT_NAME(target, "BTN_HOME",  CWIID_BTN_HOME);
  NODE_DEFINE_CONSTANT_NAME(target, "BTN_LEFT",  CWIID_BTN_LEFT);
  NODE_DEFINE_CONSTANT_NAME(target, "BTN_RIGHT", CWIID_BTN_RIGHT);
  NODE_DEFINE_CONSTANT_NAME(target, "BTN_UP",    CWIID_BTN_UP);
  NODE_DEFINE_CONSTANT_NAME(target, "BTN_DOWN",  CWIID_BTN_DOWN);

  NODE_DEFINE_CONSTANT_NAME(target, "EXT_NONE",       CWIID_EXT_NONE);
  NODE_DEFINE_CONSTANT_NAME(target, "EXT_NUNCHUK",    CWIID_EXT_NUNCHUK);
  NODE_DEFINE_CONSTANT_NAME(target, "EXT_CLASSIC",    CWIID_EXT_CLASSIC);
  NODE_DEFINE_CONSTANT_NAME(target, "EXT_BALANCE",    CWIID_EXT_BALANCE);
  NODE_DEFINE_CONSTANT_NAME(target, "EXT_MOTIONPLUS", CWIID_EXT_MOTIONPLUS);
  NODE_DEFINE_CONSTANT_NAME(target, "EXT_GUITAR",     CWIID_EXT_GUITAR);
  NODE_DEFINE_CONSTANT_NAME(target, "EXT_DRUMS",      CWIID_EXT_DRUMS);
  NODE_DEFINE_CONSTANT_NAME(target, "EXT_TURNTABLES", CWIID_EXT_TURNTABLES);
  NODE_DEFINE_CONSTANT_NAME(target, "EXT_UNKNOWN",    CWIID_EXT_UNKNOWN);

  NODE_DEFINE_CONSTANT_NAME(target, "ERROR_NONE",       CWIID_ERROR_NONE);
  NODE_DEFINE_CONSTANT_NAME(target, "ERROR_DISCONNECT", CWIID_ERROR_DISCONNECT);
  NODE_DEFINE_CONSTANT_NAME(target, "ERROR_COMM",       CWIID_ERROR_COMM);

  target->Set(String::NewSymbol("WiiMote"), constructor_template->GetFunction());
}

int WiiMote::Connect(bdaddr_t * mac) {
  DEBUG_OPT("Connecting to %s", batostr(mac));
  bacpy(&this->mac, mac);

  if(!(this->wiimote = cwiid_open(&this->mac, CWIID_FLAG_MESG_IFC))) {
    return -1;
  }

  return 0;
}

int WiiMote::Disconnect() {
  if (this->wiimote) {

    if (cwiid_get_data(this->wiimote)) {
      cwiid_set_mesg_callback(this->wiimote, NULL);
      cwiid_set_data(this->wiimote, NULL);
    }

    cwiid_close(this->wiimote);
    this->wiimote = NULL;
  }

  return 0;
}

int WiiMote::Rumble(bool on) {
  unsigned char rumble = on ? 1 : 0;

  assert(this->wiimote != NULL);

  if(cwiid_set_rumble(this->wiimote, rumble)) {
    return -1;
  }
  
  return 0;
}

int WiiMote::Led(int index, bool on) {
  int indexes[] = { CWIID_LED1_ON, CWIID_LED2_ON, CWIID_LED3_ON, CWIID_LED4_ON };

  assert(this->wiimote != NULL);

  if(cwiid_get_state(this->wiimote, &this->state)) {
    return -1;
  }

  int led = this->state.led;

  led = on ? led | indexes[index-1] : led & indexes[index-1];

  if(cwiid_set_led(this->wiimote, led)) {
    return -1;
  }

  return 0;
}

/**
 * Turns on or off the particular modes passed
 */
int WiiMote::Reporting(int mode, bool on) {
  assert(this->wiimote != NULL);

  if(cwiid_get_state(this->wiimote, &this->state)) {
    return -1;
  }

  int newmode = this->state.rpt_mode;

  newmode = on ? (newmode | mode) : (newmode & ~mode);

  if(cwiid_set_rpt_mode(this->wiimote, newmode)) {
    return -1;
  }

  return 0;
}

void WiiMote::HandleAccMessage(struct timespec *ts, cwiid_acc_mesg * msg) {
  HandleScope scope;

  Local<Object> pos = Object::New();   // Create array of x,y,z
  pos->Set(String::NewSymbol("x"), Integer::New(msg->acc[CWIID_X]) );
  pos->Set(String::NewSymbol("y"), Integer::New(msg->acc[CWIID_Y]) );
  pos->Set(String::NewSymbol("z"), Integer::New(msg->acc[CWIID_Z]) );

  Local<Value> argv[1] = { pos };
  this->Emit(acc_event, 1, argv);
}

void WiiMote::HandleButtonMessage(struct timespec *ts, cwiid_btn_mesg * msg) {
  HandleScope scope;

  Local<Integer> btn = Integer::New(msg->buttons);

  Local<Value> argv[1] = { btn };
  this->Emit(button_event, 1, argv);
}

void WiiMote::HandleErrorMessage(struct timespec *ts, cwiid_error_mesg * msg) {
  HandleScope scope;

  Local<Integer> err = Integer::New(msg->error);

  Local<Value> argv[1] = { err };
  this->Emit(error_event, 1, argv);
}

void WiiMote::HandleNunchukMessage(struct timespec *ts, cwiid_nunchuk_mesg * msg) {

}

void WiiMote::HandleIRMessage(struct timespec *ts, cwiid_ir_mesg * msg) {
  HandleScope scope;
  Local<Array> poss = Array::New(CWIID_IR_SRC_COUNT);

  // Check IR data sources
  for(int i=0; i < CWIID_IR_SRC_COUNT; i++) {
    if (!msg->src[i].valid)
      break; // Once we find one invalid then we stop

    // Create array of x,y
    Local<Object> pos = Object::New();
    pos->Set(String::NewSymbol("x"), Integer::New( msg->src[i].pos[CWIID_X] ));
    pos->Set(String::NewSymbol("y"), Integer::New( msg->src[i].pos[CWIID_Y] ));
    pos->Set(String::NewSymbol("size"), Integer::New( msg->src[i].size ));

    poss->Set(Integer::New(i), pos);
  }

  Local<Value> argv[1] = { poss };
  this->Emit(ir_event, 1, argv);
}

void WiiMote::HandleStatusMessage(struct timespec *ts, cwiid_status_mesg * msg) {
  HandleScope scope;

  Local<Object> obj = Object::New();

  obj->Set(String::NewSymbol("battery"),    Integer::New(msg->battery));
  obj->Set(String::NewSymbol("extensions"), Integer::New(msg->ext_type));

  Local<Value> argv[1] = { obj };
  this->Emit(status_event, 1, argv);
}

int WiiMote::HandleMessagesAfter(eio_req *req) {
  message_request* r = static_cast<message_request* >(req->data);
  WiiMote * self = r->wiimote;

  for (int i = 0; i < r->len; i++) {
    switch(r->mesgs[i].type) {
      case CWIID_MESG_STATUS:
        self->HandleStatusMessage(&r->timestamp, (cwiid_status_mesg *)&r->mesgs[i]);
        break;

      case CWIID_MESG_BTN:
        self->HandleButtonMessage(&r->timestamp, (cwiid_btn_mesg *)&r->mesgs[i]);
        break;

      case CWIID_MESG_ACC:
        self->HandleAccMessage(&r->timestamp, (cwiid_acc_mesg *)&r->mesgs[i]);
        break;

      case CWIID_MESG_IR:
        self->HandleIRMessage(&r->timestamp, (cwiid_ir_mesg *)&r->mesgs[i]);
        break;

      case CWIID_MESG_NUNCHUK:
        self->HandleNunchukMessage(&r->timestamp, (cwiid_nunchuk_mesg *)&r->mesgs[i]);
        break;

      case CWIID_MESG_ERROR:
        self->HandleErrorMessage(&r->timestamp, (cwiid_error_mesg *)&r->mesgs[i]);
        break;

      case CWIID_MESG_UNKNOWN:
      case CWIID_MESG_CLASSIC:
      case CWIID_MESG_BALANCE:
      case CWIID_MESG_MOTIONPLUS:
      default:
        break;
    }
  }

  free(r);
  return 0;
}

void WiiMote::HandleMessages(cwiid_wiimote_t *wiimote, int len, union cwiid_mesg mesgs[], struct timespec *timestamp) {
  WiiMote *self = const_cast<WiiMote*>(static_cast<const WiiMote*>(cwiid_get_data(wiimote)));

  // There is a race condition where this might happen
  if (self == NULL)
    return;

  struct message_request * req = (struct message_request *)malloc( sizeof(*req) + sizeof(req->mesgs) * (len - 1) );
  req->wiimote = self;
  req->timestamp = *timestamp;
  req->len = len;
  memcpy(req->mesgs, mesgs, len * sizeof(union cwiid_mesg));

  // We need to pass this over to the nodejs thread, so it can create V8 objects
  // TODO figure out if there is a better way to put an event directly on the main queue/thread.
  eio_nop (EIO_PRI_DEFAULT, WiiMote::HandleMessagesAfter, req);
}

Handle<Value> WiiMote::New(const Arguments& args) {
  HandleScope scope;

  WiiMote* wiimote = new WiiMote();
  wiimote->Wrap(args.This());

  return args.This();
}

Handle<Value> WiiMote::Connect(const Arguments& args) {
  WiiMote* wiimote = ObjectWrap::Unwrap<WiiMote>(args.This());
  Local<Function> callback;

  HandleScope scope;

  if(args.Length() == 0 || !args[0]->IsString()) {
    return ThrowException(Exception::Error(String::New("MAC address is required and must be a String.")));
  }

  if(args.Length() == 1 || !args[1]->IsFunction()) {
    return ThrowException(Exception::Error(String::New("Callback is required and must be a Function.")));
  }

  callback = Local<Function>::Cast(args[1]);

  connect_request* ar = new connect_request();
  ar->wiimote = wiimote;

  String::Utf8Value mac(args[0]);
  str2ba(*mac, &ar->mac); // TODO Validate the mac and throw an exception if invalid

  ar->callback = Persistent<Function>::New(callback);

  wiimote->Ref();

  eio_custom(EIO_Connect, EIO_PRI_DEFAULT, EIO_AfterConnect, ar);
  ev_ref(EV_DEFAULT_UC);

  return Undefined();
}

void WiiMote::EIO_Connect(eio_req* req) {
  connect_request* ar = static_cast<connect_request* >(req->data);

  assert(ar->wiimote != NULL);

  ar->err = ar->wiimote->Connect(&ar->mac);
}

int WiiMote::EIO_AfterConnect(eio_req* req) {
  HandleScope scope;

  connect_request* ar = static_cast<connect_request* >(req->data);

  WiiMote * wiimote = ar->wiimote;
  Local<Value> argv[1] = { Integer::New(ar->err) };

  if (ar->err == 0) {
    // Setup the callback to receive events
    cwiid_set_data(wiimote->wiimote, wiimote);
    cwiid_set_mesg_callback(wiimote->wiimote, WiiMote::HandleMessages);
  }

  ev_unref(EV_DEFAULT_UC);
  wiimote->Unref();

  TryCatch try_catch;

  ar->callback->Call(Context::GetCurrent()->Global(), 1, argv);

  if(try_catch.HasCaught())
    FatalException(try_catch);

  ar->callback.Dispose();

  delete ar;

  return 0;
}

Handle<Value> WiiMote::Disconnect(const Arguments& args) {
  HandleScope scope;

  WiiMote* wiimote = ObjectWrap::Unwrap<WiiMote>(args.This());
  return Integer::New(wiimote->Disconnect());
}


Handle<Value> WiiMote::Rumble(const Arguments& args) {
  HandleScope scope;

  WiiMote* wiimote = ObjectWrap::Unwrap<WiiMote>(args.This());

  if(args.Length() == 0 || !args[0]->IsBoolean()) {
    return ThrowException(Exception::Error(String::New("On state is required and must be a Boolean.")));
  }

  bool on = args[0]->ToBoolean()->Value();

  return Integer::New(wiimote->Rumble(on));
}

Handle<Value> WiiMote::Led(const Arguments& args) {
  HandleScope scope;

  WiiMote* wiimote = ObjectWrap::Unwrap<WiiMote>(args.This());

  if(args.Length() == 0 || !args[0]->IsNumber()) {
    return ThrowException(Exception::Error(String::New("Index is required and must be a Number.")));
  }

  if(args.Length() == 1 || !args[1]->IsBoolean()) {
    return ThrowException(Exception::Error(String::New("On state is required and must be a Boolean.")));
  }

  int index = args[0]->ToInteger()->Value();
  bool on = args[1]->ToBoolean()->Value();

  return Integer::New(wiimote->Led(index, on));
}


Handle<Value> WiiMote::IrReporting(const Arguments& args) {
  HandleScope scope;

  WiiMote* wiimote = ObjectWrap::Unwrap<WiiMote>(args.This());

  if(args.Length() == 0 || !args[0]->IsBoolean()) {
    return ThrowException(Exception::Error(String::New("On state is required and must be a Boolean.")));
  }

  bool on = args[0]->ToBoolean()->Value();
  return Integer::New(wiimote->Reporting(CWIID_RPT_IR, on));
}

Handle<Value> WiiMote::AccReporting(const Arguments& args) {
  HandleScope scope;

  WiiMote* wiimote = ObjectWrap::Unwrap<WiiMote>(args.This());

  if(args.Length() == 0 || !args[0]->IsBoolean()) {
    return ThrowException(Exception::Error(String::New("On state is required and must be a Boolean.")));
  }

  bool on = args[0]->ToBoolean()->Value();
  return Integer::New(wiimote->Reporting(CWIID_RPT_ACC, on));
}

Handle<Value> WiiMote::ExtReporting(const Arguments& args) {
  HandleScope scope;

  WiiMote* wiimote = ObjectWrap::Unwrap<WiiMote>(args.This());

  if(args.Length() == 0 || !args[0]->IsBoolean()) {
    return ThrowException(Exception::Error(String::New("On state is required and must be a Boolean.")));
  }

  bool on = args[0]->ToBoolean()->Value();
  return Integer::New(wiimote->Reporting(CWIID_RPT_EXT, on));
}

Handle<Value> WiiMote::ButtonReporting(const Arguments& args) {
  HandleScope scope;

  WiiMote* wiimote = ObjectWrap::Unwrap<WiiMote>(args.This());

  if(args.Length() == 0 || !args[0]->IsBoolean()) {
    return ThrowException(Exception::Error(String::New("On state is required and must be a Boolean.")));
  }

  bool on = args[0]->ToBoolean()->Value();

  return Integer::New(wiimote->Reporting(CWIID_RPT_BTN, on));
}

Persistent<FunctionTemplate> WiiMote::constructor_template;
Persistent<String> WiiMote::ir_event;
Persistent<String> WiiMote::acc_event;
Persistent<String> WiiMote::nunchuk_event;
Persistent<String> WiiMote::error_event;
Persistent<String> WiiMote::button_event;
Persistent<String> WiiMote::status_event;
