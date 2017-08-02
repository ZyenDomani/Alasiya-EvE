#ifndef EVE_MAIL_H
#define EVE_MAIL_H

enum MailStatusMask {
  mailStatusMaskRead = 1,
  mailStatusMaskReplied = 2,
  mailStatusMaskForwarded = 4,
  mailStatusMaskTrashed = 8,
  mailStatusMaskDraft = 16,
  mailStatusMaskAutomated = 32,
};

enum mailLabelMask {
  mailLabelInbox = 1,
  mailLabelSent = 2,
  mailLabelCorporation = 4,
  mailLabelAlliance = 8,
  mailLabelsSystem = mailLabelInbox + mailLabelSent + mailLabelCorporation + mailLabelAlliance,
};


#define mailingListBlocked 0
#define mailingListAllowed 1
#define mailingListMemberMuted 0
#define mailingListMemberDefault 1
#define mailingListMemberOperator 2
#define mailingListMemberOwner 3


#define mailMaxRecipients 50
#define mailMaxGroups 1
#define mailMaxSubjectSize 150
#define mailMaxBodySize 8000
#define mailMaxTaggedBodySize 10000
#define mailMaxLabelSize 40
#define mailMaxNumLabels 25
#define mailMaxPerPage 100
#define mailTrialAccountTimer 1
#define mailMaxMessagePerMinute 5
#define mailinglistMaxMembers 3000
#define mailinglistMaxMembersUpdated 1000
#define mailingListMaxNameSize 60



#endif
