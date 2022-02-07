# Ryzom Core [![Build Status](https://dev.azure.com/ryzom/ryzomcore/_apis/build/status/ryzom.ryzomcore?branchName=core4)](https://dev.azure.com/ryzom/ryzomcore/_build)

Ryzom Core is the open-source project related to Ryzom Game. Written in C++, Ryzom Core contains the whole code (client, server, tools) used to make the commercial MMORPG Ryzom. Ryzom Core is a toolkit for the development of massively multiplayer online universes. It provides the base technologies and a set of development methodologies for the development of both client and server code.

Ryzom Core is open source and released under the terms of the GNU Affero General Public License 3.0 (GNU/AGPLv3) for the source code. The art assets are dual-licenced under the Creative Commons Attributions-ShareAlike 3.0 (CC-BY-SA) and Free Art License 1.3 (FAL 1.3).

* Wiki: https://wiki.ryzom.dev/
* IRC: https://freegamedev.net/irc/#ryzom
* Discord: https://discord.gg/xjSBVkSmCy

## About the community

We are a passionate group of gamers, and open-source aficionados. Many of us fell in love with the original vision and promise of the Saga of Ryzom, or the ideals of open source. For some the game was once our home, or still is. To others, it was an introduction to the open-source software world. We work on Ryzom Core because we care for the things we love, and the friendships we've made over the years.

The Ryzom Core repository is maintained by the community independently. It is not officially affiliated with the commercial game, nor with Winch Gate Ltd. See the History section on why.

## Support development

We are happy to work on this project in our free time. If you would like to contribute to the project financially, you can support us and our loved ones through any of the following links.

* [![Sponsor](https://img.shields.io/github/sponsors/kaetemi?style=social)]( https://github.com/sponsors/kaetemi) Jan Boon (Kaetemi), Polyverse OÜ. — Goes towards the Ryzom Core servers, and open-source development.

If you're a regular source code or game asset contributor to the open-source project, feel free to add yourself to this list.

## Project history

The Saga of Ryzom was originally developed by Nevrax. A small game development company with a visionary creative direction.

Several founders and developers of the company at the time were open-source aficionados. The ideal in their minds was to fully open source the game. Unfortunately, the investors thought differently and never fully bought into the open-source model. Only the engine was released at the time.

This gave birth to the NeL project. The Nevrax Library. While there was public interest in the engine, the lack of commitment to open source by the company made it difficult for independent contributors to work on the project.

When Nevrax eventually went bankrupt, they were initially bought out by Gameforge. A subsidiary called Gameforge France was formed to work on Ryzom.

A community-led [Virtual Citizenship Association (VCA)]( https://web.archive.org/web/20080905175524/https://www.virtualcitizenship.org/), raising over € 170,000 in pledges, failed to win the bid. The open-source NeL community was later abandoned by Gameforge once Ryzom eventually fell into oblivion, and revived itself as the OpenNeL project. After Gameforge failed to fully pay for the bankruptcy deal, and some hush-hush behind the backs of the community-led VCA, the ownership was transferred back to the liquidator and onto Winch Gate Properties Limited.

Fortunately, thanks to the efforts of original Nevrax employees, and key members of OpenNeL, the new owner showed some goodwill towards the open source community, and the full game client and server, as well as a substantial amount of graphics assets, were released as open source.

With the promise of the game becoming an open source project, they worked together with the OpenNeL community, which renamed to Ryzom Core in order to advertise the game better. In practice, the commercial game servers were being developed on a private repository. Beyond bug fixes and enhancements to the client, the community had zero input. New features that required server modifications were disallowed. The collaboration was doomed to fail.

The Ryzom Core community was backstabbed and slowly ignored by the new owner. A new Ryzom Forge project was launched behind closed doors, that claimed to make community contributions easier. Yet closing off development almost entirely to outsiders. No attempt was made by the new owner at fully committing to an open-source development model.

New features on the commercial game developed by the Ryzom Forge team are primarily web applications using the in-game browser, circumventing the AGPLv3. Furthermore, the Ryzom Forge team had at one point updated their private server repository to link the binaries to the game engine on the public repository. This in violation of the AGPLv3 license on community contributions, forcing their hand to make their private server fork public. The web apps remain closed source. The team also shut down the Ryzom Ring player scenario tools on the official servers, in favor of closed source web-based tools that are exclusive to the official event team.

As the commercial game owners historically, and still, have been a poorly committed and unreliable partner to running this as a truly open-source project, Ryzom Core is currently an independent community project, and is committed to remain that way and protect the rights of independent contributors.

## Copyright and contributions

All original source code is copyright (C) 2001-2022 Winch Gate Property Limited. The commercial game's official Git repositories are hosted on GitLab at https://gitlab.com/ryzom.

The NeL Sound driver for XAudio2 is copyright (C) 2008-2014 Jan BOON <jan.boon@kaetemi.be>.

Other community contributions and modifications are copyright their respective authors. Consult the OpenNeL SVN and the Ryzom Core Git repository histories for more details.

The source code in this repository is licensed under the AGPLv3, unless specified otherwise. Likewise, by committing to this repository, you agree to license your modifications under the AGPLv3. You retain all copyright over your work (\*). As such, you have the option to additionally dual-license your fully-owned contributions under any other license. Configuration files are implied to be public domain samples, as they may contain sensitive data in a production environment.

(\*) If, and only if, you have signed a CTA (part of the NDA, copyright and ownership - your contract may vary) with Winch Gate Ltd., your contributions, along with any other creations related to Ryzom, fall under that contract and will be copyright Winch Gate Ltd. instead (\*\*). As per your contract, your contributions are licensed back to you under the AGPLv3. You retain the moral rights to your work. These are non-transferable rights, which permit you to assert your authorship and be credited for your contributions. You should exercise this right.

(\*\*) If you live in a legislation where contractual copyright assignments without employment or commercial transaction are unenforceable (e.g. Belgium), and you are not being paid by Winch Gate Ltd. for your current work, the CTA is null and void, and you retain full copyright ownership over your contributions. Consult a lawyer to assert your rights. Don't give them away.

To comply with the AGPLv3 license, section 5(a), names of the contributors who own the copyrights to modifications will be added to the copyright notice in source files on a periodic basis. You may follow the existing format if you wish to do this yourself.

The removal of any valid copyright notice is a violation of the AGPLv3 license.
