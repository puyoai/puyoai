#include <gtest/gtest.h>

#include "db.h"

using namespace std;

TEST(DB, normalizeSeq) {
  EXPECT_EQ(
    normalizeSeq("CD-DA-DA-"),
    "AB-AC-AC-");
  EXPECT_EQ(
    normalizeSeq("AB-CD-CD-"),
    "AB-CD-CD-");
  EXPECT_EQ(
    normalizeSeq("DC-DA-CA-"),
    "AB-AC-BC-");
  EXPECT_EQ(
    normalizeSeq("AB-CC-AB-BB-DD-AA-BC-AD-CC-BD-AD-AC-"),
    "AB-CC-AB-AA-DD-BB-AC-BD-CC-AD-BD-BC-");
  EXPECT_EQ(
    normalizeSeq("CD-DA-DA-AA-CA-CC-CA-BC-DD-BA-BA-AB-DB-AB-BD-DD-CC-CC-AA-"),
    "AB-AC-AC-CC-BC-BB-BC-BD-AA-CD-CD-CD-AD-CD-AD-AA-BB-BB-CC-");
  EXPECT_EQ(
    normalizeSeq("DD-BD-BD-CC-BB-DB-BD-CB-CC-CA-CC-BC-DC-BA-BA-BB-CB-DC-CC-"),
    "AA-AB-AB-CC-BB-AB-AB-BC-CC-CD-CC-BC-AC-BD-BD-BB-BC-AC-CC-");
}

TEST(DB, normalizeSeqUni) {
  EXPECT_EQ(
    normalizeSeqUni("CD-DA-DA-"),
    "AB-AC-AC-");
  EXPECT_EQ(
    normalizeSeqUni("AB-CD-CD-"),
    "AB-CD-CD-");
  EXPECT_EQ(
    normalizeSeqUni("DC-DA-CA-"),
    "AB-AC-BC-");
  EXPECT_EQ(
    normalizeSeqUni("AB-CC-AB-BB-DD-AA-BC-AD-CC-BD-AD-AC-"),
    "AB-CC-AB-AA-DD-BB-AC-BD-CC-AD-BD-BC-");
  EXPECT_EQ(
    normalizeSeqUni(
      "CD-DA-DA-AA-CA-CC-CA-BC-DD-BA-BA-AB-DB-AB-BD-DD-CC-CC-AA-"),
    "AB-AC-AC-CC-BC-BB-BC-BD-AA-CD-CD-CD-AD-CD-AD-AA-BB-BB-CC-");
  EXPECT_EQ(
    normalizeSeqUni(
      "DD-BD-BD-CC-BB-DB-BD-CB-CC-CA-CC-BC-DC-BA-BA-BB-CB-DC-CC-"),
    "AA-AB-AB-CC-BB-AB-AB-BC-CC-CD-CC-BC-AC-BD-BD-BB-BC-AC-CC-");
}
