#include <node.h>
#include <rfb/rfb.h>

using namespace node;
using namespace v8;


#define REQ_ARGS(N)                                                   \
  if (args.Length() < N)                                              \
    return ThrowException(                                            \
      Exception::TypeError(String::New("Expected " #N " arguments"))  \
    );

#define REQ_INT_ARG(I, VAR)                             \
  int VAR;                                              \
  if (args.Length() <= I || !args[I]->IsInt32())        \
    return ThrowException(Exception::TypeError(         \
      String::New("Argument " #I " must be an integer") \
    ));                                                 \
  VAR = args[I]->Int32Value();

#define REQ_STR_ARG(I, VAR)                             \
  if (args.Length() <= I || !args[I]->IsString())       \
    return ThrowException(Exception::TypeError(         \
      String::New("Argument " #I " must be a string"))  \
  );                                                    \
  String::Utf8Value VAR(args[I]->ToString());


class RfbScreen : public ObjectWrap {
 public:
  static Persistent<FunctionTemplate> constructor;

  static void Init(Handle<Object> target) {
    HandleScope scope;

    Local<FunctionTemplate> tmpl = FunctionTemplate::New(New);
    Local<String> name = String::NewSymbol("RfbClient");

    constructor = Persistent<FunctionTemplate>::New(tmpl);
    constructor->InstanceTemplate()->SetInternalFieldCount(1);
    constructor->SetClassName(name);

    NODE_SET_PROTOTYPE_METHOD(tmpl, "close", Close);
    NODE_SET_PROTOTYPE_METHOD(tmpl, "listen", Listen);

    target->Set(name, constructor->GetFunction());
  }

 protected:
  static Handle<Value> Close(const Arguments& args) {
    HandleScope scope;

    RfbScreen* screen = ObjectWrap::Unwrap<RfbScreen>(args.This());

    rfbShutdownServer(screen->info_, TRUE);

    ev_io_stop(EV_DEFAULT_UC_ &screen->watcher_);
    screen->Unref();

    return scope.Close(Undefined());
  }

  static Handle<Value> Listen(const Arguments& args) {
    HandleScope scope;

    REQ_ARGS(2)
    REQ_INT_ARG(0, port)
    REQ_STR_ARG(1, host)

    RfbScreen* screen = ObjectWrap::Unwrap<RfbScreen>(args.This());

    screen->info_->listenInterface = inet_addr(*host);
    screen->info_->port = port;
    rfbInitServer(screen->info_);

    screen->watcher_.data = screen;
    ev_init(&screen->watcher_, OnData);
    ev_io_set(&screen->watcher_, screen->info_->listenSock, EV_READ);
    ev_io_start(EV_DEFAULT_UC_ &screen->watcher_);
    screen->Ref();

    return scope.Close(Undefined());
  }

  static Handle<Value> New(const Arguments& args) {
    HandleScope scope;

    REQ_ARGS(2)
    REQ_INT_ARG(0, width)
    REQ_INT_ARG(1, height)

    RfbScreen* screen = new RfbScreen(width, height);
    screen->Wrap(args.This());

    return args.This();
  }

  static enum rfbNewClientAction NewClient(rfbClientPtr cl) {
    HandleScope scope;

    RfbScreen* screen = static_cast<RfbScreen*>(cl->screen->screenData);

    rfbProcessEvents(screen->info_, 0);

    //Handle<Value> client = Client::New(cl);
    //screen->Emit(client_symbol, 1, &client);
    //MakeCallback(screen, "onclient", 0, NULL);

    return RFB_CLIENT_ACCEPT;
  }

  static void OnData(EV_P_ ev_io *w, int events) {
    RfbScreen* screen = static_cast<RfbScreen*>(w->data);
    rfbProcessEvents(screen->info_, 0);
  }

  RfbScreen(int width, int height) {
    info_ = rfbGetScreen(0, NULL, width, height, 8, 3, 4);
    info_->cursor = NULL;
    info_->frameBuffer = (char*) malloc(width * height * 4);
    info_->newClientHook = NewClient;
    info_->screenData = (void*) this;
  }

  ~RfbScreen() {
    free(info_->frameBuffer);
    rfbScreenCleanup(info_);
  }

  rfbScreenInfoPtr info_;
  ev_io watcher_;
};


static void init(Handle<Object> target) {
  RfbScreen::Init(target);
}
NODE_MODULE(binding, init);
