#include <inductions.hpp>
#include <set>
#include <algorithm>

namespace eden
{
   void inductions::initialize_induction(uint64_t id,
                                         eosio::name inviter,
                                         eosio::name invitee,
                                         const std::vector<eosio::name>& witnesses)
   {
      check_new_induction(invitee, inviter);
      check_valid_endorsers(inviter, witnesses);

      induction_tb.emplace(contract, [&](auto& row) {
         row.id = id;
         row.inviter = inviter;
         row.invitee = invitee;
         row.witnesses = witnesses;
         row.endorsements = {};
         row.created_at = eosio::current_block_time();
         row.video = "";
         row.new_member_profile = {};
      });
   }

   void inductions::update_profile(const induction& induction,
                                   const new_member_profile& new_member_profile)
   {
      check_valid_induction(induction);
      validate_profile(new_member_profile);

      induction_tb.modify(induction_tb.iterator_to(induction), eosio::same_payer, [&](auto& row) {
         row.new_member_profile = new_member_profile;
         row.endorsements = {};
      });
   }

   void inductions::check_valid_endorsers(eosio::name inviter,
                                          const std::vector<eosio::name>& witnesses) const
   {
      eosio::check(witnesses.size() >= 2 && witnesses.size() <= 5,
                   "2 to 5 witnesses are required for induction");

      std::set<eosio::name> unique_witnesses_set(witnesses.begin(), witnesses.end());
      eosio::check(unique_witnesses_set.size() == witnesses.size(),
                   "the witnesses list has a duplicated entry");

      eosio::check(!unique_witnesses_set.contains(inviter),
                   "inviter cannot be is in the witnesses list");
   }

   void inductions::check_new_induction(eosio::name invitee, eosio::name inviter) const
   {
      auto invitee_index = induction_tb.get_index<"byinvitee"_n>();

      auto invitee_key = combine_names(invitee, inviter);
      auto itr = invitee_index.find(invitee_key);

      eosio::check(itr == invitee_index.end(),
                   "induction for this invitation is already in progress");
   }

   const induction& inductions::get_induction(uint64_t id) const
   {
      return induction_tb.get(id, "unable to find induction");
   }

   void inductions::check_valid_induction(const induction& induction) const
   {
      auto induction_lifetime = eosio::current_time_point() - induction.created_at.to_time_point();
      eosio::check(induction_lifetime.to_seconds() <= induction_expiration_secs,
                   "induction has expired");
   }

   void inductions::update_video(const induction& induction,
				 const std::string& video)
   {
      check_valid_induction(induction);
      validate_video(video);

      induction_tb.modify(induction_tb.iterator_to(induction), eosio::same_payer, [&](auto& row) {
         row.video = video;
         row.endorsements = {};
      });
   }

   void inductions::validate_profile(const new_member_profile& new_member_profile) const
   {
      eosio::check(!new_member_profile.name.empty(), "new member profile name is empty");
      eosio::check(!new_member_profile.img.empty(), "new member profile img is empty");
      eosio::check(!new_member_profile.bio.empty(), "new member profile bio is empty");
      // TODO: add more checks (valid ipfs img, bio and name minimum length)
   }

   void inductions::validate_video(const std::string& video) const
   {
      // TODO: check that video is a valid IPFS CID.
   }
}  // namespace eden
