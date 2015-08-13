# **Contributing to Wi-Fi Test Suite**

**Thank you for your interest in contributing to the Wi-Fi Alliance's (WFA) Wi-Fi Test Suite project!**

## 1. Become a Contributer

Contributors are members who contribute to the code, testing, bug reporting, documentation, design, discussion etc. Contributors get involved in the project through the issue tracker, forum, and the contributor mailing list. Changes are submitted through pull requests which will be considered for inclusion in the project by Committers.

How to become one:
Contributors must sign the WFA Contributor Agreement before contributing work to WFA Wi-Fi Test Suite software thru the following URL:http://www.wi-fi.org/wi-fi-alliance-contributor-agreement

Contributors must follow the Contribution Standards, which is in sections 2 through 6 of this file. Submit a pull request. We'll evaluate the request to determine if it aligns with the projects goals. In addition to the goal alignment there is also a requirement to include appropriate tests and these tests run without failure. If all of the requirements are met we’ll accept it and merge it into the project.

## 2. Propose a Submission

Proposals can be submitted via a WFA Wi-Fi Test Suite Blog or as an idea to the Contributor Mailing List. Submissions that follow our contribution standards and are aligned with the direction of the project will be accepted. Please review Wi-Fi Test Suite contributor model for more information about how proposals get processed once submitted.

## 3. Minimum Requirements

The goal of these requirements is to ensure quality, API consistency, code reusability, and maintainability of the contribution. All contributions must satisfy:

- Complete API Docs and inline code comments
- Unit tests with 90% line coverage
- User Guide (Components only)
- Proper commit logs
- Proper purpose of the functionality

## 4. Further Recommendations

We recommend the following process for contributing great code to Wi-Fi Test Suite:

4.1 Diff Before Every Commit

	Get into the habit of running git diff or git diff --cached before every commit.

4.2. Commits Should Be Granular

	You should keep each commit as granular as possible. For instance, do not check in 2 bug fixes in one commit -- separate them out into 2 commits. Any big change must be split into multiple commit as small feature

4.3. Coding Style

	Please make sure your changes conform to Style Guide for Python Code (PEP8).
	Or download pylint and run on python scripts: http://www.pylint.org/#install.

4.4. Get feedback early and often

	- A draft about the proposal.
	- An API review validates your initial approach.
	- A design review validates your high-level architecture.
	- A code review validates your implementation details.

4.5. Documentation is just as important as code

	- API docs
	- Examples
	- User guides

4.6. Do your tests

	- Unit
	- Functional
	- Performance

## 5. License Updates

Any submission that uses open source code from another project must be explicitly called out and is subject to review. Any proprietary code from any project, should not be submitted, and will be rejected.

## 6. Right to Revert

Once the contribution has been merged into the repo, if any issues arise in the integration environment or upon subsequent feedback, the contribution may be reverted.