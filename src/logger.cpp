/* This file is part of the the dci project. Copyright (C) 2013-2022 vopl, shtoba.
   This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public
   License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
   You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>. */

#include "pch.hpp"
#include "logger.hpp"

namespace std
{
    using namespace dci::module::ppn::node;

    inline std::ostream& operator<<(std::ostream& o, const api::link::Remote<>& r)
    {
        (void) r;
        return o;//<<"r";
    }

    inline std::ostream& operator<<(std::ostream& o, const transport::Address& a)
    {
        return o<<a.value;
    }

    inline std::ostream& operator<<(std::ostream& o, const api::link::Id& id)
    {
        o<<dci::utils::b2h(id.data(), id.size());
        return o;
    }

    inline std::ostream& operator<<(std::ostream& o, const ExceptionPtr& e)
    {
        return o<<dci::exception::toString(e);
    }
}

namespace dci::module::ppn::node
{
    namespace
    {
        /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
        std::string addr2String(const transport::Address& a1, const transport::Address& a2)
        {
            if(!a1.value.empty() && !a2.value.empty())
            {
                if(a1 == a2)
                {
                    return a1.value;
                }

                return a1.value + " (" + a2.value + ")";
            }

            return a1.value + a2.value;
        }

        /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
        bool parseBool(const String& param)
        {
            static const std::regex t("^(t|true|on|enable|allow|1|)$", std::regex_constants::icase | std::regex::optimize);
            return std::regex_match(param, t);
        }

    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Logger::Logger()
        : api::Logger<>::Opposite(idl::interface::Initializer{})
    {
        //link
        {
            api::link::Feature<>::Opposite op = *this;

            //in setup(feature::Service);
            op->setup() += sol() * [this](api::link::feature::Service<> srv)
            {
                //out joinedByConnect(Remote);
                srv->joinedByConnect() += sol() * [this](const api::link::Id& id, const api::link::Remote<>& r)
                {
                    if(test("link", "remote", "joinedByConnect"))
                    {
                        r->remoteAddress().then() += sol() * [id](auto in)
                        {
                            if(in.resolvedValue())
                            {
                                LOGI("remote joinedByConnect"<<": "<<id<<", address: "<<in.detachValue());
                            }
                            else if(in.resolvedException())
                            {
                                LOGI("remote joinedByConnect"<<": "<<id<<", address obtaining failed: "<<in.exception());
                            }
                            else// if(in.resolvedCancel())
                            {
                                LOGI("remote joinedByConnect"<<": "<<id<<", address obtaining cancelled");
                            }
                        };
                    }

                    r->closed() += sol() * [id,this]()
                    {
                        if(test("link", "remote", "closed")) LOGI("remote closed: "<<id);
                    };
                    r->failed() += sol() * [id,this](const ExceptionPtr& e)
                    {
                        if(test("link", "remote", "failed")) LOGI("remote failed: "<<id<<", "<<e);
                    };
                };

                //out joinedByAccept(Remote);
                srv->joinedByAccept() += sol() * [this](const api::link::Id& id, const api::link::Remote<>& r)
                {
                    if(test("link", "remote", "joinedByAccept"))
                    {
                        r->remoteAddress().then() += sol() * [id](auto in)
                        {
                            if(in.resolvedValue())
                            {
                                LOGI("remote joinedByAccept"<<": "<<id<<", address: "<<in.detachValue());
                            }
                            else if(in.resolvedException())
                            {
                                LOGI("remote joinedByAccept"<<": "<<id<<", address obtaining failed: "<<in.exception());
                            }
                            else// if(in.resolvedCancel())
                            {
                                LOGI("remote joinedByAccept"<<": "<<id<<", address obtaining cancelled");
                            }
                        };
                    }

                    r->closed() += sol() * [id,this]()
                    {
                        if(test("link", "remote", "closed")) LOGI("remote closed: "<<id);
                    };
                    r->failed() += sol() * [id,this](const ExceptionPtr& e)
                    {
                        if(test("link", "remote", "failed")) LOGI("remote failed: "<<id<<", "<<e);
                    };
                };

                if(test("link", "local", "id"))
                {
                    srv->id().then() += sol() * [](auto in)
                    {
                        if(in.resolvedValue())
                        {
                            LOGI("local link id: "<<in.value());
                        }
                        else if(in.resolvedException())
                        {
                            LOGW("local link id obtaining failed: "<<in.exception());
                        }
                        else// if(in.resolvedCancel())
                        {
                            LOGW("local link id obtaining cancelled");
                        }
                    };
                }
            };
        }

        //node
        {
            api::Feature<>::Opposite op = *this;

            //in setup(feature::Service);
            op->setup() += sol() * [this](api::feature::Service<> srv)
            {
                //out start();
                srv->start() += sol() * [this]()
                {
                    if(test("start")) LOGI("start");
                };

                //out stop();
                srv->stop() += sol() * [this]()
                {
                    if(test("stop")) LOGI("stop");
                };

                //out failed(exception);
                srv->failed() += sol() * [this](const ExceptionPtr& e)
                {
                    if(test("failed")) LOGI("failed: "<<e);
                };

                {
                    api::feature::RemoteAddressSpace<> ras = srv;

                    //out discovered(link::Id, transport::Address);
                    ras->discovered() += sol() * [this](const api::link::Id& id, const transport::Address& a)
                    {
                        if(test("ras", "discovered")) LOGI("discovered: "<<id<<", "<<a);
                    };
                }

                {
                    api::feature::LocalAddressSpace<> las = srv;

                    //out declared(transport::Address);
                    las->declared() += sol() * [this](const transport::Address& a)
                    {
                        if(test("las", "declared")) LOGI("declared: "<<a);
                    };

                    //out undeclared(transport::Address);
                    las->undeclared() += sol() * [this](const transport::Address& a)
                    {
                        if(test("las", "undeclared")) LOGI("undeclared: "<<a);
                    };
                }

                {
                    api::feature::Connectors<> csrv = srv;

                    //out connectorStarted(transport::Address);
                    csrv->connectorStarted() += sol() * [this](const transport::Address& a)
                    {
                        if(test("connector", "started")) LOGI("connectorStarted: "<<a);
                    };

                    //out connectorStopped(transport::Address);
                    csrv->connectorStopped() += sol() * [this](const transport::Address& a)
                    {
                        if(test("connector", "stopped")) LOGI("connectorStopped: "<<a);
                    };

                    //out newSession(link::Id, transport::Address, CSession);
                    csrv->newSession() += sol() * [this](const api::link::Id& id, const transport::Address& a, api::feature::CSession<> s) mutable
                    {
                        static uint64 cidGen = 0;
                        uint64 cid = ++cidGen;

                        if(test("connector", "session", "new")) LOGI("new csession "<<cid<<", id: "<<id<<", address: "<<a);

                        //out connected();
                        s->connected() += sol() * [cid,this]
                        {
                            if(test("connector", "session", "connected")) LOGI("csession "<<cid<<" connected");
                        };

                        //out idSpecified(link::Id);
                        s->idSpecified() += sol() * [cid,this](const api::link::Id& id)
                        {
                            if(test("connector", "session", "idSpecified")) LOGI("csession "<<cid<<" idSpecified: "<<id);
                        };

                        //out failed(exception);
                        s->failed() += sol() * [cid,this](ExceptionPtr e)
                        {
                            if(test("connector", "session", "failed")) LOGI("csession "<<cid<<" failed: "<<e);
                        };

                        //out joined();
                        s->joined() += sol() * [cid,this](const api::link::Remote<>&)
                        {
                            if(test("connector", "session", "joined")) LOGI("csession "<<cid<<" joined");
                        };

                        //out closed();
                        s->closed() += sol() * [cid,this]
                        {
                            if(test("connector", "session", "closed")) LOGI("csession "<<cid<<" closed");
                        };
                    };
                }

                {
                    api::feature::Acceptors<> asrv = srv;

                    //out acceptorStarted(transport::Address);
                    asrv->acceptorStarted() += sol() * [this](const transport::Address& a1, const transport::Address& a2)
                    {
                        if(test("acceptor", "started")) LOGI("acceptorStarted: "<<addr2String(a1, a2));
                    };

                    //out acceptorStopped(transport::Address);
                    asrv->acceptorStopped() += sol() * [this](const transport::Address& a1, const transport::Address& a2)
                    {
                        if(test("acceptor", "stopped")) LOGI("acceptorStopped: "<<addr2String(a1, a2));
                    };

                    //out acceptorFailed(transport::Address, exception);
                    asrv->acceptorFailed() += sol() * [this](const transport::Address& a1, const transport::Address& a2, const ExceptionPtr& e)
                    {
                        if(test("acceptor", "failed")) LOGI("acceptorFailed: "<<addr2String(a1, a2)<<", "<<e);
                    };

                    //out newSession(transport::Address, ASession);
                    asrv->newSession() += sol() * [this](api::feature::ASession<> s) mutable
                    {
                        static uint64 cidGen = 0;
                        uint64 cid = ++cidGen;

                        if(test("acceptor", "session", "new"))
                        {
                            s->address().then() += sol() * [cid](auto in)
                            {
                                if(in.resolvedValue())
                                {
                                    LOGI("new asession "<<cid<<", address: "<<in.detachValue());
                                }
                                else if(in.resolvedException())
                                {
                                    LOGW("new asession "<<cid<<", address obtaining failed: "<<in.exception());
                                }
                                else// if(in.resolvedCancel())
                                {
                                    LOGW("new asession "<<cid<<", address obtaining cancelled");
                                }
                            };
                        }

                        //out idSpecified(link::Id);
                        s->idSpecified() += sol() * [cid,this](const api::link::Id& id)
                        {
                            if(test("acceptor", "session", "idSpecified")) LOGI("asession "<<cid<<" idSpecified: "<<id);
                        };

                        //out failed(exception);
                        s->failed() += sol() * [cid,this](ExceptionPtr e)
                        {
                            if(test("acceptor", "session", "failed")) LOGI("asession "<<cid<<" failed: "<<e);
                        };

                        //out joined();
                        s->joined() += sol() * [cid,this](const api::link::Remote<>&)
                        {
                            if(test("acceptor", "session", "joined")) LOGI("asession "<<cid<<" joined");
                        };

                        //out closed();
                        s->closed() += sol() * [cid,this]
                        {
                            if(test("acceptor", "session", "closed")) LOGI("asession "<<cid<<" closed");
                        };
                    };
                }
            };
        }

        //in configure(Config) -> void;
        methods()->configure() += sol() * [this](idl::Config&& config)
        {
            _config.clear();
            std::function<void(const String&, const config::ptree&)> traverse = [&,this](const String& prefix, const config::ptree& pt)
            {
                for(const auto&[k, v] : pt)
                {
                    const String& akey = prefix.empty() ? k : prefix+"."+k;
                    _config[utils::fnv1a(akey)] = parseBool(v.data());
                    traverse(akey, v);
                }
            };
            traverse({}, config::cnvt(std::move(config)));

            return cmt::readyFuture(None{});
        };
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Logger::~Logger()
    {
        sol().flush();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    bool Logger::test(const auto& key)
    {
        auto iter = _config.find(utils::fnv1a(key));
        if(_config.end() == iter)
        {
            return true;
        }

        return iter->second;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    bool Logger::test(const auto& prefix, const auto& head, const auto&... tail)
    {
        if(!test(prefix))
        {
            return false;
        }

        char prefix2[sizeof(prefix) + sizeof(head)];

        std::copy(&prefix[0], &prefix[sizeof(prefix)-1], &prefix2[0]);
        prefix2[sizeof(prefix)-1] = '.';
        std::copy(&head[0], &head[sizeof(head)], &prefix2[sizeof(prefix)]);

        return test(prefix2, tail...);
    }
}
