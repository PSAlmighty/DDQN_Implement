// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: DQN_Agent.proto

#ifndef PROTOBUF_DQN_5fAgent_2eproto__INCLUDED
#define PROTOBUF_DQN_5fAgent_2eproto__INCLUDED

#include <string>

#include <google/protobuf/stubs/common.h>

#if GOOGLE_PROTOBUF_VERSION < 3005000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please update
#error your headers.
#endif
#if 3005001 < GOOGLE_PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/arena.h>
#include <google/protobuf/arenastring.h>
#include <google/protobuf/generated_message_table_driven.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/metadata.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>  // IWYU pragma: export
#include <google/protobuf/extension_set.h>  // IWYU pragma: export
#include <google/protobuf/unknown_field_set.h>
// @@protoc_insertion_point(includes)

namespace protobuf_DQN_5fAgent_2eproto {
// Internal implementation detail -- do not use these members.
struct TableStruct {
  static const ::google::protobuf::internal::ParseTableField entries[];
  static const ::google::protobuf::internal::AuxillaryParseTableField aux[];
  static const ::google::protobuf::internal::ParseTable schema[2];
  static const ::google::protobuf::internal::FieldMetadata field_metadata[];
  static const ::google::protobuf::internal::SerializationTable serialization_table[];
  static const ::google::protobuf::uint32 offsets[];
};
void AddDescriptors();
void InitDefaultsAgent_InfoImpl();
void InitDefaultsAgent_Info();
void InitDefaultsMD_InfoImpl();
void InitDefaultsMD_Info();
inline void InitDefaults() {
  InitDefaultsAgent_Info();
  InitDefaultsMD_Info();
}
}  // namespace protobuf_DQN_5fAgent_2eproto
class Agent_Info;
class Agent_InfoDefaultTypeInternal;
extern Agent_InfoDefaultTypeInternal _Agent_Info_default_instance_;
class MD_Info;
class MD_InfoDefaultTypeInternal;
extern MD_InfoDefaultTypeInternal _MD_Info_default_instance_;

// ===================================================================

class Agent_Info : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:Agent_Info) */ {
 public:
  Agent_Info();
  virtual ~Agent_Info();

  Agent_Info(const Agent_Info& from);

  inline Agent_Info& operator=(const Agent_Info& from) {
    CopyFrom(from);
    return *this;
  }
  #if LANG_CXX11
  Agent_Info(Agent_Info&& from) noexcept
    : Agent_Info() {
    *this = ::std::move(from);
  }

  inline Agent_Info& operator=(Agent_Info&& from) noexcept {
    if (GetArenaNoVirtual() == from.GetArenaNoVirtual()) {
      if (this != &from) InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }
  #endif
  static const ::google::protobuf::Descriptor* descriptor();
  static const Agent_Info& default_instance();

  static void InitAsDefaultInstance();  // FOR INTERNAL USE ONLY
  static inline const Agent_Info* internal_default_instance() {
    return reinterpret_cast<const Agent_Info*>(
               &_Agent_Info_default_instance_);
  }
  static PROTOBUF_CONSTEXPR int const kIndexInFileMessages =
    0;

  void Swap(Agent_Info* other);
  friend void swap(Agent_Info& a, Agent_Info& b) {
    a.Swap(&b);
  }

  // implements Message ----------------------------------------------

  inline Agent_Info* New() const PROTOBUF_FINAL { return New(NULL); }

  Agent_Info* New(::google::protobuf::Arena* arena) const PROTOBUF_FINAL;
  void CopyFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void MergeFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void CopyFrom(const Agent_Info& from);
  void MergeFrom(const Agent_Info& from);
  void Clear() PROTOBUF_FINAL;
  bool IsInitialized() const PROTOBUF_FINAL;

  size_t ByteSizeLong() const PROTOBUF_FINAL;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input) PROTOBUF_FINAL;
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const PROTOBUF_FINAL;
  ::google::protobuf::uint8* InternalSerializeWithCachedSizesToArray(
      bool deterministic, ::google::protobuf::uint8* target) const PROTOBUF_FINAL;
  int GetCachedSize() const PROTOBUF_FINAL { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const PROTOBUF_FINAL;
  void InternalSwap(Agent_Info* other);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return NULL;
  }
  inline void* MaybeArenaPtr() const {
    return NULL;
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const PROTOBUF_FINAL;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // string Agent_Trading_Instrument = 3;
  void clear_agent_trading_instrument();
  static const int kAgentTradingInstrumentFieldNumber = 3;
  const ::std::string& agent_trading_instrument() const;
  void set_agent_trading_instrument(const ::std::string& value);
  #if LANG_CXX11
  void set_agent_trading_instrument(::std::string&& value);
  #endif
  void set_agent_trading_instrument(const char* value);
  void set_agent_trading_instrument(const char* value, size_t size);
  ::std::string* mutable_agent_trading_instrument();
  ::std::string* release_agent_trading_instrument();
  void set_allocated_agent_trading_instrument(::std::string* agent_trading_instrument);

  // string Action_Timestamp = 4;
  void clear_action_timestamp();
  static const int kActionTimestampFieldNumber = 4;
  const ::std::string& action_timestamp() const;
  void set_action_timestamp(const ::std::string& value);
  #if LANG_CXX11
  void set_action_timestamp(::std::string&& value);
  #endif
  void set_action_timestamp(const char* value);
  void set_action_timestamp(const char* value, size_t size);
  ::std::string* mutable_action_timestamp();
  ::std::string* release_action_timestamp();
  void set_allocated_action_timestamp(::std::string* action_timestamp);

  // fixed32 Agent_Action = 1;
  void clear_agent_action();
  static const int kAgentActionFieldNumber = 1;
  ::google::protobuf::uint32 agent_action() const;
  void set_agent_action(::google::protobuf::uint32 value);

  // fixed32 Current_Position = 2;
  void clear_current_position();
  static const int kCurrentPositionFieldNumber = 2;
  ::google::protobuf::uint32 current_position() const;
  void set_current_position(::google::protobuf::uint32 value);

  // bool LatestResult = 5;
  void clear_latestresult();
  static const int kLatestResultFieldNumber = 5;
  bool latestresult() const;
  void set_latestresult(bool value);

  // bool msg_received = 6;
  void clear_msg_received();
  static const int kMsgReceivedFieldNumber = 6;
  bool msg_received() const;
  void set_msg_received(bool value);

  // @@protoc_insertion_point(class_scope:Agent_Info)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  ::google::protobuf::internal::ArenaStringPtr agent_trading_instrument_;
  ::google::protobuf::internal::ArenaStringPtr action_timestamp_;
  ::google::protobuf::uint32 agent_action_;
  ::google::protobuf::uint32 current_position_;
  bool latestresult_;
  bool msg_received_;
  mutable int _cached_size_;
  friend struct ::protobuf_DQN_5fAgent_2eproto::TableStruct;
  friend void ::protobuf_DQN_5fAgent_2eproto::InitDefaultsAgent_InfoImpl();
};
// -------------------------------------------------------------------

class MD_Info : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:MD_Info) */ {
 public:
  MD_Info();
  virtual ~MD_Info();

  MD_Info(const MD_Info& from);

  inline MD_Info& operator=(const MD_Info& from) {
    CopyFrom(from);
    return *this;
  }
  #if LANG_CXX11
  MD_Info(MD_Info&& from) noexcept
    : MD_Info() {
    *this = ::std::move(from);
  }

  inline MD_Info& operator=(MD_Info&& from) noexcept {
    if (GetArenaNoVirtual() == from.GetArenaNoVirtual()) {
      if (this != &from) InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }
  #endif
  static const ::google::protobuf::Descriptor* descriptor();
  static const MD_Info& default_instance();

  static void InitAsDefaultInstance();  // FOR INTERNAL USE ONLY
  static inline const MD_Info* internal_default_instance() {
    return reinterpret_cast<const MD_Info*>(
               &_MD_Info_default_instance_);
  }
  static PROTOBUF_CONSTEXPR int const kIndexInFileMessages =
    1;

  void Swap(MD_Info* other);
  friend void swap(MD_Info& a, MD_Info& b) {
    a.Swap(&b);
  }

  // implements Message ----------------------------------------------

  inline MD_Info* New() const PROTOBUF_FINAL { return New(NULL); }

  MD_Info* New(::google::protobuf::Arena* arena) const PROTOBUF_FINAL;
  void CopyFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void MergeFrom(const ::google::protobuf::Message& from) PROTOBUF_FINAL;
  void CopyFrom(const MD_Info& from);
  void MergeFrom(const MD_Info& from);
  void Clear() PROTOBUF_FINAL;
  bool IsInitialized() const PROTOBUF_FINAL;

  size_t ByteSizeLong() const PROTOBUF_FINAL;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input) PROTOBUF_FINAL;
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const PROTOBUF_FINAL;
  ::google::protobuf::uint8* InternalSerializeWithCachedSizesToArray(
      bool deterministic, ::google::protobuf::uint8* target) const PROTOBUF_FINAL;
  int GetCachedSize() const PROTOBUF_FINAL { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const PROTOBUF_FINAL;
  void InternalSwap(MD_Info* other);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return NULL;
  }
  inline void* MaybeArenaPtr() const {
    return NULL;
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const PROTOBUF_FINAL;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // string MD_Timestamp = 4;
  void clear_md_timestamp();
  static const int kMDTimestampFieldNumber = 4;
  const ::std::string& md_timestamp() const;
  void set_md_timestamp(const ::std::string& value);
  #if LANG_CXX11
  void set_md_timestamp(::std::string&& value);
  #endif
  void set_md_timestamp(const char* value);
  void set_md_timestamp(const char* value, size_t size);
  ::std::string* mutable_md_timestamp();
  ::std::string* release_md_timestamp();
  void set_allocated_md_timestamp(::std::string* md_timestamp);

  // double last_price = 1;
  void clear_last_price();
  static const int kLastPriceFieldNumber = 1;
  double last_price() const;
  void set_last_price(double value);

  // fixed32 volume = 2;
  void clear_volume();
  static const int kVolumeFieldNumber = 2;
  ::google::protobuf::uint32 volume() const;
  void set_volume(::google::protobuf::uint32 value);

  // bool msg_received = 3;
  void clear_msg_received();
  static const int kMsgReceivedFieldNumber = 3;
  bool msg_received() const;
  void set_msg_received(bool value);

  // @@protoc_insertion_point(class_scope:MD_Info)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  ::google::protobuf::internal::ArenaStringPtr md_timestamp_;
  double last_price_;
  ::google::protobuf::uint32 volume_;
  bool msg_received_;
  mutable int _cached_size_;
  friend struct ::protobuf_DQN_5fAgent_2eproto::TableStruct;
  friend void ::protobuf_DQN_5fAgent_2eproto::InitDefaultsMD_InfoImpl();
};
// ===================================================================


// ===================================================================

#ifdef __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif  // __GNUC__
// Agent_Info

// fixed32 Agent_Action = 1;
inline void Agent_Info::clear_agent_action() {
  agent_action_ = 0u;
}
inline ::google::protobuf::uint32 Agent_Info::agent_action() const {
  // @@protoc_insertion_point(field_get:Agent_Info.Agent_Action)
  return agent_action_;
}
inline void Agent_Info::set_agent_action(::google::protobuf::uint32 value) {
  
  agent_action_ = value;
  // @@protoc_insertion_point(field_set:Agent_Info.Agent_Action)
}

// fixed32 Current_Position = 2;
inline void Agent_Info::clear_current_position() {
  current_position_ = 0u;
}
inline ::google::protobuf::uint32 Agent_Info::current_position() const {
  // @@protoc_insertion_point(field_get:Agent_Info.Current_Position)
  return current_position_;
}
inline void Agent_Info::set_current_position(::google::protobuf::uint32 value) {
  
  current_position_ = value;
  // @@protoc_insertion_point(field_set:Agent_Info.Current_Position)
}

// string Agent_Trading_Instrument = 3;
inline void Agent_Info::clear_agent_trading_instrument() {
  agent_trading_instrument_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& Agent_Info::agent_trading_instrument() const {
  // @@protoc_insertion_point(field_get:Agent_Info.Agent_Trading_Instrument)
  return agent_trading_instrument_.GetNoArena();
}
inline void Agent_Info::set_agent_trading_instrument(const ::std::string& value) {
  
  agent_trading_instrument_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:Agent_Info.Agent_Trading_Instrument)
}
#if LANG_CXX11
inline void Agent_Info::set_agent_trading_instrument(::std::string&& value) {
  
  agent_trading_instrument_.SetNoArena(
    &::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::move(value));
  // @@protoc_insertion_point(field_set_rvalue:Agent_Info.Agent_Trading_Instrument)
}
#endif
inline void Agent_Info::set_agent_trading_instrument(const char* value) {
  GOOGLE_DCHECK(value != NULL);
  
  agent_trading_instrument_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:Agent_Info.Agent_Trading_Instrument)
}
inline void Agent_Info::set_agent_trading_instrument(const char* value, size_t size) {
  
  agent_trading_instrument_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:Agent_Info.Agent_Trading_Instrument)
}
inline ::std::string* Agent_Info::mutable_agent_trading_instrument() {
  
  // @@protoc_insertion_point(field_mutable:Agent_Info.Agent_Trading_Instrument)
  return agent_trading_instrument_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* Agent_Info::release_agent_trading_instrument() {
  // @@protoc_insertion_point(field_release:Agent_Info.Agent_Trading_Instrument)
  
  return agent_trading_instrument_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void Agent_Info::set_allocated_agent_trading_instrument(::std::string* agent_trading_instrument) {
  if (agent_trading_instrument != NULL) {
    
  } else {
    
  }
  agent_trading_instrument_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), agent_trading_instrument);
  // @@protoc_insertion_point(field_set_allocated:Agent_Info.Agent_Trading_Instrument)
}

// string Action_Timestamp = 4;
inline void Agent_Info::clear_action_timestamp() {
  action_timestamp_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& Agent_Info::action_timestamp() const {
  // @@protoc_insertion_point(field_get:Agent_Info.Action_Timestamp)
  return action_timestamp_.GetNoArena();
}
inline void Agent_Info::set_action_timestamp(const ::std::string& value) {
  
  action_timestamp_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:Agent_Info.Action_Timestamp)
}
#if LANG_CXX11
inline void Agent_Info::set_action_timestamp(::std::string&& value) {
  
  action_timestamp_.SetNoArena(
    &::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::move(value));
  // @@protoc_insertion_point(field_set_rvalue:Agent_Info.Action_Timestamp)
}
#endif
inline void Agent_Info::set_action_timestamp(const char* value) {
  GOOGLE_DCHECK(value != NULL);
  
  action_timestamp_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:Agent_Info.Action_Timestamp)
}
inline void Agent_Info::set_action_timestamp(const char* value, size_t size) {
  
  action_timestamp_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:Agent_Info.Action_Timestamp)
}
inline ::std::string* Agent_Info::mutable_action_timestamp() {
  
  // @@protoc_insertion_point(field_mutable:Agent_Info.Action_Timestamp)
  return action_timestamp_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* Agent_Info::release_action_timestamp() {
  // @@protoc_insertion_point(field_release:Agent_Info.Action_Timestamp)
  
  return action_timestamp_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void Agent_Info::set_allocated_action_timestamp(::std::string* action_timestamp) {
  if (action_timestamp != NULL) {
    
  } else {
    
  }
  action_timestamp_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), action_timestamp);
  // @@protoc_insertion_point(field_set_allocated:Agent_Info.Action_Timestamp)
}

// bool LatestResult = 5;
inline void Agent_Info::clear_latestresult() {
  latestresult_ = false;
}
inline bool Agent_Info::latestresult() const {
  // @@protoc_insertion_point(field_get:Agent_Info.LatestResult)
  return latestresult_;
}
inline void Agent_Info::set_latestresult(bool value) {
  
  latestresult_ = value;
  // @@protoc_insertion_point(field_set:Agent_Info.LatestResult)
}

// bool msg_received = 6;
inline void Agent_Info::clear_msg_received() {
  msg_received_ = false;
}
inline bool Agent_Info::msg_received() const {
  // @@protoc_insertion_point(field_get:Agent_Info.msg_received)
  return msg_received_;
}
inline void Agent_Info::set_msg_received(bool value) {
  
  msg_received_ = value;
  // @@protoc_insertion_point(field_set:Agent_Info.msg_received)
}

// -------------------------------------------------------------------

// MD_Info

// double last_price = 1;
inline void MD_Info::clear_last_price() {
  last_price_ = 0;
}
inline double MD_Info::last_price() const {
  // @@protoc_insertion_point(field_get:MD_Info.last_price)
  return last_price_;
}
inline void MD_Info::set_last_price(double value) {
  
  last_price_ = value;
  // @@protoc_insertion_point(field_set:MD_Info.last_price)
}

// fixed32 volume = 2;
inline void MD_Info::clear_volume() {
  volume_ = 0u;
}
inline ::google::protobuf::uint32 MD_Info::volume() const {
  // @@protoc_insertion_point(field_get:MD_Info.volume)
  return volume_;
}
inline void MD_Info::set_volume(::google::protobuf::uint32 value) {
  
  volume_ = value;
  // @@protoc_insertion_point(field_set:MD_Info.volume)
}

// bool msg_received = 3;
inline void MD_Info::clear_msg_received() {
  msg_received_ = false;
}
inline bool MD_Info::msg_received() const {
  // @@protoc_insertion_point(field_get:MD_Info.msg_received)
  return msg_received_;
}
inline void MD_Info::set_msg_received(bool value) {
  
  msg_received_ = value;
  // @@protoc_insertion_point(field_set:MD_Info.msg_received)
}

// string MD_Timestamp = 4;
inline void MD_Info::clear_md_timestamp() {
  md_timestamp_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& MD_Info::md_timestamp() const {
  // @@protoc_insertion_point(field_get:MD_Info.MD_Timestamp)
  return md_timestamp_.GetNoArena();
}
inline void MD_Info::set_md_timestamp(const ::std::string& value) {
  
  md_timestamp_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:MD_Info.MD_Timestamp)
}
#if LANG_CXX11
inline void MD_Info::set_md_timestamp(::std::string&& value) {
  
  md_timestamp_.SetNoArena(
    &::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::move(value));
  // @@protoc_insertion_point(field_set_rvalue:MD_Info.MD_Timestamp)
}
#endif
inline void MD_Info::set_md_timestamp(const char* value) {
  GOOGLE_DCHECK(value != NULL);
  
  md_timestamp_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:MD_Info.MD_Timestamp)
}
inline void MD_Info::set_md_timestamp(const char* value, size_t size) {
  
  md_timestamp_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:MD_Info.MD_Timestamp)
}
inline ::std::string* MD_Info::mutable_md_timestamp() {
  
  // @@protoc_insertion_point(field_mutable:MD_Info.MD_Timestamp)
  return md_timestamp_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* MD_Info::release_md_timestamp() {
  // @@protoc_insertion_point(field_release:MD_Info.MD_Timestamp)
  
  return md_timestamp_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void MD_Info::set_allocated_md_timestamp(::std::string* md_timestamp) {
  if (md_timestamp != NULL) {
    
  } else {
    
  }
  md_timestamp_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), md_timestamp);
  // @@protoc_insertion_point(field_set_allocated:MD_Info.MD_Timestamp)
}

#ifdef __GNUC__
  #pragma GCC diagnostic pop
#endif  // __GNUC__
// -------------------------------------------------------------------


// @@protoc_insertion_point(namespace_scope)


// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_DQN_5fAgent_2eproto__INCLUDED
