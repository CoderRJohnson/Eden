#pragma once

#include <constants.hpp>
#include <eosio/asset.hpp>
#include <eosio/eosio.hpp>
#include <string>
#include <utils.hpp>

namespace eden
{
   struct new_member_profile
   {
      std::string name;
      std::string img;
      std::string bio;
      std::string social;
   };
   EOSIO_REFLECT(new_member_profile, name, img, bio)

   struct induction
   {
      uint64_t id;
      eosio::name inviter;
      eosio::name invitee;
      std::vector<eosio::name> witnesses;
      std::vector<eosio::name> endorsements;
      eosio::block_timestamp created_at;
      std::string video;
      new_member_profile new_member_profile;

      uint64_t primary_key() const { return id; }
      uint128_t get_invitee_inviter() const { return combine_names(invitee, inviter); }
   };
   EOSIO_REFLECT(induction,
                 id,
                 inviter,
                 invitee,
                 witnesses,
                 endorsements,
                 created_at,
                 video,
                 new_member_profile)

   using induction_table_type = eosio::multi_index<
       "induction"_n,
       induction,
       eosio::indexed_by<
           "byinvitee"_n,
           eosio::const_mem_fun<induction, uint128_t, &induction::get_invitee_inviter>>>;

   class inductions
   {
     private:
      eosio::name contract;
      induction_table_type induction_tb;

      void check_new_induction(eosio::name invitee, eosio::name inviter) const;
      void check_valid_induction(const induction& induction) const;
      void validate_profile(const new_member_profile& new_member_profile) const;
      void validate_video(const std::string& video) const;
      void check_valid_endorsers(eosio::name inviter,
                                 const std::vector<eosio::name>& witnesses) const;

     public:
      inductions(eosio::name contract) : contract(contract), induction_tb(contract, default_scope)
      {
      }

      const induction& get_induction(uint64_t id) const;

      void initialize_induction(uint64_t id,
                                eosio::name inviter,
                                eosio::name invitee,
                                const std::vector<eosio::name>& witnesses);

      void update_profile(const induction& induction, const new_member_profile& new_member_profile);

      void update_video(const induction& induction,
		        const std::string& video);
   };

}  // namespace eden
