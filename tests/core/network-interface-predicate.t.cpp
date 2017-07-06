/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2017,  Regents of the University of California,
 *                           Arizona Board of Regents,
 *                           Colorado State University,
 *                           University Pierre & Marie Curie, Sorbonne University,
 *                           Washington University in St. Louis,
 *                           Beijing Institute of Technology,
 *                           The University of Memphis.
 *
 * This file is part of NFD (Named Data Networking Forwarding Daemon).
 * See AUTHORS.md for complete list of NFD authors and contributors.
 *
 * NFD is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * NFD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * NFD, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "core/network-interface-predicate.hpp"
#include "core/network-interface.hpp"

#include "tests/test-common.hpp"

#include <ndn-cxx/net/network-monitor-stub.hpp>
#include <boost/property_tree/info_parser.hpp>
#include <sstream>

namespace nfd {
namespace tests {

class NetworkInterfacePredicateFixture : public BaseFixture
{
protected:
  NetworkInterfacePredicateFixture()
  {
    using namespace boost::asio::ip;
    interfaces.push_back(
      NetworkInterfaceInfo{0, "eth0",
        ethernet::Address::fromString("3e:15:c2:8b:65:00"),
        {address_v4::from_string("129.82.100.1")},
        {},
        address_v4::from_string("129.82.255.255"),
        IFF_UP});
    interfaces.push_back(
      NetworkInterfaceInfo{1, "eth1",
        ethernet::Address::fromString("3e:15:c2:8b:65:01"),
        {address_v4::from_string("192.168.2.1")},
        {},
        address_v4::from_string("192.168.2.255"),
        IFF_UP});
    interfaces.push_back(
      NetworkInterfaceInfo{2, "eth2",
        ethernet::Address::fromString("3e:15:c2:8b:65:02"),
        {address_v4::from_string("198.51.100.1")},
        {address_v6::from_string("2001:db8::1")},
        address_v4::from_string("198.51.100.255"),
        IFF_MULTICAST | IFF_BROADCAST | IFF_UP});
    interfaces.push_back(
      NetworkInterfaceInfo{3, "enp68s0f1",
        ethernet::Address::fromString("3e:15:c2:8b:65:03"),
        {address_v4::from_string("192.168.2.3")},
        {},
        address_v4::from_string("192.168.2.255"),
        IFF_UP});

    using namespace ndn::net;

    netifs.push_back(NetworkMonitorStub::makeNetworkInterface());
    netifs.back()->setIndex(0);
    netifs.back()->setName("eth0");
    netifs.back()->setEthernetAddress(ethernet::Address::fromString("3e:15:c2:8b:65:00"));
    netifs.back()->addNetworkAddress(NetworkAddress(AddressFamily::V4,
      address_v4::from_string("129.82.100.1"), address_v4::from_string("129.82.255.255"),
      16, AddressScope::GLOBAL, 0));
    netifs.back()->setFlags(IFF_UP);

    netifs.push_back(NetworkMonitorStub::makeNetworkInterface());
    netifs.back()->setIndex(1);
    netifs.back()->setName("eth1");
    netifs.back()->setEthernetAddress(ethernet::Address::fromString("3e:15:c2:8b:65:01"));
    netifs.back()->addNetworkAddress(NetworkAddress(AddressFamily::V4,
      address_v4::from_string("192.168.2.1"), address_v4::from_string("192.168.2.255"),
      24, AddressScope::GLOBAL, 0));
    netifs.back()->setFlags(IFF_UP);

    netifs.push_back(NetworkMonitorStub::makeNetworkInterface());
    netifs.back()->setIndex(2);
    netifs.back()->setName("eth2");
    netifs.back()->setEthernetAddress(ethernet::Address::fromString("3e:15:c2:8b:65:02"));
    netifs.back()->addNetworkAddress(NetworkAddress(AddressFamily::V4,
      address_v4::from_string("198.51.100.1"), address_v4::from_string("198.51.100.255"),
      24, AddressScope::GLOBAL, 0));
    netifs.back()->addNetworkAddress(NetworkAddress(AddressFamily::V6,
      address_v6::from_string("2001:db8::1"), address_v6::from_string("2001:db8::ffff"),
      112, AddressScope::GLOBAL, 0));
    netifs.back()->setFlags(IFF_MULTICAST | IFF_BROADCAST | IFF_UP);

    netifs.push_back(NetworkMonitorStub::makeNetworkInterface());
    netifs.back()->setIndex(3);
    netifs.back()->setName("enp68s0f1");
    netifs.back()->setEthernetAddress(ethernet::Address::fromString("3e:15:c2:8b:65:03"));
    netifs.back()->addNetworkAddress(NetworkAddress(AddressFamily::V4,
      address_v4::from_string("192.168.2.3"), address_v4::from_string("192.168.2.255"),
      24, AddressScope::GLOBAL, 0));
    netifs.back()->setFlags(IFF_UP);
  }

  void
  parseConfig(const std::string& config)
  {
    std::istringstream input(config);
    boost::property_tree::ptree ptree;
    boost::property_tree::read_info(input, ptree);

    for (const auto& i : ptree) {
      if (i.first == "whitelist") {
        predicate.parseWhitelist(i.second);
      }
      else if (i.first == "blacklist") {
        predicate.parseBlacklist(i.second);
      }
    }
  }

protected:
  NetworkInterfacePredicate predicate;
  std::vector<NetworkInterfaceInfo> interfaces;
  std::vector<shared_ptr<ndn::net::NetworkInterface>> netifs;
};

BOOST_FIXTURE_TEST_SUITE(TestNetworkInterfacePredicate, NetworkInterfacePredicateFixture)

BOOST_AUTO_TEST_CASE(Default)
{
  parseConfig("");

  BOOST_CHECK_EQUAL(predicate(interfaces[0]), true);
  BOOST_CHECK_EQUAL(predicate(interfaces[1]), true);
  BOOST_CHECK_EQUAL(predicate(interfaces[2]), true);
  BOOST_CHECK_EQUAL(predicate(interfaces[3]), true);

  BOOST_CHECK_EQUAL(predicate(*netifs[0]), true);
  BOOST_CHECK_EQUAL(predicate(*netifs[1]), true);
  BOOST_CHECK_EQUAL(predicate(*netifs[2]), true);
  BOOST_CHECK_EQUAL(predicate(*netifs[3]), true);
}

BOOST_AUTO_TEST_CASE(EmptyWhitelist)
{
  parseConfig("whitelist\n"
              "{\n"
              "}");

  BOOST_CHECK_EQUAL(predicate(interfaces[0]), false);
  BOOST_CHECK_EQUAL(predicate(interfaces[1]), false);
  BOOST_CHECK_EQUAL(predicate(interfaces[2]), false);
  BOOST_CHECK_EQUAL(predicate(interfaces[3]), false);

  BOOST_CHECK_EQUAL(predicate(*netifs[0]), false);
  BOOST_CHECK_EQUAL(predicate(*netifs[1]), false);
  BOOST_CHECK_EQUAL(predicate(*netifs[2]), false);
  BOOST_CHECK_EQUAL(predicate(*netifs[3]), false);
}

BOOST_AUTO_TEST_CASE(WildcardBlacklist)
{
  parseConfig("blacklist\n"
              "{\n"
              "  *\n"
              "}");

  BOOST_CHECK_EQUAL(predicate(interfaces[0]), false);
  BOOST_CHECK_EQUAL(predicate(interfaces[1]), false);
  BOOST_CHECK_EQUAL(predicate(interfaces[2]), false);
  BOOST_CHECK_EQUAL(predicate(interfaces[3]), false);

  BOOST_CHECK_EQUAL(predicate(*netifs[0]), false);
  BOOST_CHECK_EQUAL(predicate(*netifs[1]), false);
  BOOST_CHECK_EQUAL(predicate(*netifs[2]), false);
  BOOST_CHECK_EQUAL(predicate(*netifs[3]), false);
}

BOOST_AUTO_TEST_CASE(IfnameWhitelist)
{
  parseConfig("whitelist\n"
              "{\n"
              "  ifname eth0\n"
              "  ifname eth1\n"
              "}");

  BOOST_CHECK_EQUAL(predicate(interfaces[0]), true);
  BOOST_CHECK_EQUAL(predicate(interfaces[1]), true);
  BOOST_CHECK_EQUAL(predicate(interfaces[2]), false);
  BOOST_CHECK_EQUAL(predicate(interfaces[3]), false);

  BOOST_CHECK_EQUAL(predicate(*netifs[0]), true);
  BOOST_CHECK_EQUAL(predicate(*netifs[1]), true);
  BOOST_CHECK_EQUAL(predicate(*netifs[2]), false);
  BOOST_CHECK_EQUAL(predicate(*netifs[3]), false);
}

BOOST_AUTO_TEST_CASE(IfnameBlacklist)
{
  parseConfig("blacklist\n"
              "{\n"
              "  ifname eth0\n"
              "  ifname eth1\n"
              "}");

  BOOST_CHECK_EQUAL(predicate(interfaces[0]), false);
  BOOST_CHECK_EQUAL(predicate(interfaces[1]), false);
  BOOST_CHECK_EQUAL(predicate(interfaces[2]), true);
  BOOST_CHECK_EQUAL(predicate(interfaces[3]), true);

  BOOST_CHECK_EQUAL(predicate(*netifs[0]), false);
  BOOST_CHECK_EQUAL(predicate(*netifs[1]), false);
  BOOST_CHECK_EQUAL(predicate(*netifs[2]), true);
  BOOST_CHECK_EQUAL(predicate(*netifs[3]), true);
}

BOOST_AUTO_TEST_CASE(IfnameWildcardStart)
{
  parseConfig("whitelist\n"
              "{\n"
              "  ifname enp*\n"
              "}");

  BOOST_CHECK_EQUAL(predicate(interfaces[0]), false);
  BOOST_CHECK_EQUAL(predicate(interfaces[1]), false);
  BOOST_CHECK_EQUAL(predicate(interfaces[2]), false);
  BOOST_CHECK_EQUAL(predicate(interfaces[3]), true);

  BOOST_CHECK_EQUAL(predicate(*netifs[0]), false);
  BOOST_CHECK_EQUAL(predicate(*netifs[1]), false);
  BOOST_CHECK_EQUAL(predicate(*netifs[2]), false);
  BOOST_CHECK_EQUAL(predicate(*netifs[3]), true);
}

BOOST_AUTO_TEST_CASE(IfnameWildcardMiddle)
{
  parseConfig("whitelist\n"
              "{\n"
              "  ifname *th*\n"
              "}");

  BOOST_CHECK_EQUAL(predicate(interfaces[0]), true);
  BOOST_CHECK_EQUAL(predicate(interfaces[1]), true);
  BOOST_CHECK_EQUAL(predicate(interfaces[2]), true);
  BOOST_CHECK_EQUAL(predicate(interfaces[3]), false);

  BOOST_CHECK_EQUAL(predicate(*netifs[0]), true);
  BOOST_CHECK_EQUAL(predicate(*netifs[1]), true);
  BOOST_CHECK_EQUAL(predicate(*netifs[2]), true);
  BOOST_CHECK_EQUAL(predicate(*netifs[3]), false);
}

BOOST_AUTO_TEST_CASE(IfnameWildcardDouble)
{
  parseConfig("whitelist\n"
              "{\n"
              "  ifname eth**\n"
              "}");

  BOOST_CHECK_EQUAL(predicate(interfaces[0]), true);
  BOOST_CHECK_EQUAL(predicate(interfaces[1]), true);
  BOOST_CHECK_EQUAL(predicate(interfaces[2]), true);
  BOOST_CHECK_EQUAL(predicate(interfaces[3]), false);

  BOOST_CHECK_EQUAL(predicate(*netifs[0]), true);
  BOOST_CHECK_EQUAL(predicate(*netifs[1]), true);
  BOOST_CHECK_EQUAL(predicate(*netifs[2]), true);
  BOOST_CHECK_EQUAL(predicate(*netifs[3]), false);
}

BOOST_AUTO_TEST_CASE(IfnameWildcardOnly)
{
  parseConfig("whitelist\n"
              "{\n"
              "  ifname *\n"
              "}");

  BOOST_CHECK_EQUAL(predicate(interfaces[0]), true);
  BOOST_CHECK_EQUAL(predicate(interfaces[1]), true);
  BOOST_CHECK_EQUAL(predicate(interfaces[2]), true);
  BOOST_CHECK_EQUAL(predicate(interfaces[3]), true);

  BOOST_CHECK_EQUAL(predicate(*netifs[0]), true);
  BOOST_CHECK_EQUAL(predicate(*netifs[1]), true);
  BOOST_CHECK_EQUAL(predicate(*netifs[2]), true);
  BOOST_CHECK_EQUAL(predicate(*netifs[3]), true);
}

BOOST_AUTO_TEST_CASE(IfnameQuestionMark)
{
  parseConfig("whitelist\n"
              "{\n"
              "  ifname eth?\n"
              "}");

  BOOST_CHECK_EQUAL(predicate(interfaces[0]), true);
  BOOST_CHECK_EQUAL(predicate(interfaces[1]), true);
  BOOST_CHECK_EQUAL(predicate(interfaces[2]), true);
  BOOST_CHECK_EQUAL(predicate(interfaces[3]), false);

  BOOST_CHECK_EQUAL(predicate(*netifs[0]), true);
  BOOST_CHECK_EQUAL(predicate(*netifs[1]), true);
  BOOST_CHECK_EQUAL(predicate(*netifs[2]), true);
  BOOST_CHECK_EQUAL(predicate(*netifs[3]), false);
}

BOOST_AUTO_TEST_CASE(IfnameMalformed)
{
  BOOST_CHECK_THROW(
    parseConfig("whitelist\n"
                "{\n"
                "  ifname\n"
                "}"),
    ConfigFile::Error);
}

BOOST_AUTO_TEST_CASE(EtherWhitelist)
{
  parseConfig("whitelist\n"
              "{\n"
              "  ether 3e:15:c2:8b:65:00\n"
              "  ether 3e:15:c2:8b:65:01\n"
              "}");

  BOOST_CHECK_EQUAL(predicate(interfaces[0]), true);
  BOOST_CHECK_EQUAL(predicate(interfaces[1]), true);
  BOOST_CHECK_EQUAL(predicate(interfaces[2]), false);
  BOOST_CHECK_EQUAL(predicate(interfaces[3]), false);

  BOOST_CHECK_EQUAL(predicate(*netifs[0]), true);
  BOOST_CHECK_EQUAL(predicate(*netifs[1]), true);
  BOOST_CHECK_EQUAL(predicate(*netifs[2]), false);
  BOOST_CHECK_EQUAL(predicate(*netifs[3]), false);
}

BOOST_AUTO_TEST_CASE(EtherBlacklist)
{
  parseConfig("blacklist\n"
              "{\n"
              "  ether 3e:15:c2:8b:65:00\n"
              "  ether 3e:15:c2:8b:65:01\n"
              "}");

  BOOST_CHECK_EQUAL(predicate(interfaces[0]), false);
  BOOST_CHECK_EQUAL(predicate(interfaces[1]), false);
  BOOST_CHECK_EQUAL(predicate(interfaces[2]), true);
  BOOST_CHECK_EQUAL(predicate(interfaces[3]), true);

  BOOST_CHECK_EQUAL(predicate(*netifs[0]), false);
  BOOST_CHECK_EQUAL(predicate(*netifs[1]), false);
  BOOST_CHECK_EQUAL(predicate(*netifs[2]), true);
  BOOST_CHECK_EQUAL(predicate(*netifs[3]), true);
}

BOOST_AUTO_TEST_CASE(EtherMalformed)
{
  BOOST_CHECK_THROW(
    parseConfig("blacklist\n"
                "{\n"
                "  ether foo\n"
                "}"),
    ConfigFile::Error);
}

BOOST_AUTO_TEST_CASE(SubnetWhitelist)
{
  parseConfig("whitelist\n"
              "{\n"
              "  subnet 192.168.0.0/16\n"
              "}");

  BOOST_CHECK_EQUAL(predicate(interfaces[0]), false);
  BOOST_CHECK_EQUAL(predicate(interfaces[1]), true);
  BOOST_CHECK_EQUAL(predicate(interfaces[2]), false);
  BOOST_CHECK_EQUAL(predicate(interfaces[3]), true);

  BOOST_CHECK_EQUAL(predicate(*netifs[0]), false);
  BOOST_CHECK_EQUAL(predicate(*netifs[1]), true);
  BOOST_CHECK_EQUAL(predicate(*netifs[2]), false);
  BOOST_CHECK_EQUAL(predicate(*netifs[3]), true);
}

BOOST_AUTO_TEST_CASE(SubnetBlacklist)
{
  parseConfig("blacklist\n"
              "{\n"
              "  subnet 192.168.0.0/16\n"
              "}");

  BOOST_CHECK_EQUAL(predicate(interfaces[0]), true);
  BOOST_CHECK_EQUAL(predicate(interfaces[1]), false);
  BOOST_CHECK_EQUAL(predicate(interfaces[2]), true);
  BOOST_CHECK_EQUAL(predicate(interfaces[3]), false);

  BOOST_CHECK_EQUAL(predicate(*netifs[0]), true);
  BOOST_CHECK_EQUAL(predicate(*netifs[1]), false);
  BOOST_CHECK_EQUAL(predicate(*netifs[2]), true);
  BOOST_CHECK_EQUAL(predicate(*netifs[3]), false);
}

BOOST_AUTO_TEST_CASE(SubnetMalformed)
{
  BOOST_CHECK_THROW(
    parseConfig("blacklist\n"
                "{\n"
                "  subnet 266.0.0.91/\n"
                "}"),
    ConfigFile::Error);
}

BOOST_AUTO_TEST_SUITE_END() // TestNetworkInterfacePredicate

} // namespace tests
} // namespace nfd
