# Chip Your Luck — Firmware Repository

Welcome! This repo holds the firmware for the "Chip Your Luck" trade-show game. This README is written for anyone on the team who has **never used Git before** — by the end of it, you should be able to get code from this repo, make a change, and share it back with the team without breaking anything.

If you get stuck at any point, that's normal. Ask in `#firmware` before you spend more than 15 minutes stuck on a Git problem — nearly every Git mistake is fixable.

---

## Table of Contents

1. [What Git and GitHub Actually Are](#1-what-git-and-github-actually-are)
2. [One-Time Setup](#2-one-time-setup)
3. [Our Branch Strategy](#3-our-branch-strategy)
4. [The Golden Rule](#4-the-golden-rule)
5. [Your Daily Workflow, Step by Step](#5-your-daily-workflow-step-by-step)
6. [Git Command Cheat Sheet](#6-git-command-cheat-sheet)
7. [Common Problems & How to Fix Them](#7-common-problems--how-to-fix-them)
8. [Getting Help](#8-getting-help)

---

## 1. What Git and GitHub Actually Are

**Git** is a tool that saves snapshots of your code over time, so you can:
- See exactly what changed, when, and who changed it
- Undo mistakes without losing everything
- Work on the same project as 4 other people without overwriting each other's work

Think of it like a video game's save-file system, except every "save" (called a **commit**) is labeled with a note about what you did, and you can jump back to any save point whenever you want.

**GitHub** is a website that hosts a copy of your Git project "in the cloud" so the whole team can pull from and push to the same place. Git is the tool; GitHub is where our shared copy lives.

A few words you'll see constantly:

| Term | What it means |
|---|---|
| **Repository ("repo")** | The project folder, tracked by Git — this whole codebase |
| **Clone** | Downloading a full copy of the repo to your computer for the first time |
| **Commit** | A saved snapshot of your changes, with a short message describing them |
| **Branch** | A separate, parallel version of the code you can edit without affecting everyone else's version |
| **Push** | Uploading your commits from your computer to GitHub |
| **Pull** | Downloading other people's commits from GitHub to your computer |
| **Pull Request (PR)** | A request to merge your branch into another branch, with a chance for a teammate to review it first |
| **Merge** | Combining two branches together |
| **Merge conflict** | When Git can't automatically combine two changes because they touched the same lines — you resolve it by hand |

---

## 2. One-Time Setup

Do this once, on your own laptop.

### Step 1 — Install Git
- **Windows**: download from [git-scm.com](https://git-scm.com/downloads), run the installer, keep default options.
- **Mac**: open Terminal and type `git --version` — if it's not installed, macOS will prompt you to install it.
- **Linux**: `sudo apt install git` (Ubuntu/Debian) or your distro's equivalent.

Verify it worked by opening a terminal (Command Prompt, Terminal, or Git Bash on Windows) and typing:
```bash
git --version
```
You should see something like `git version 2.43.0`.

### Step 2 — Create a GitHub account
If you don't already have one, sign up at [github.com](https://github.com). Ask whoever set up the repo to add your GitHub username as a collaborator.

### Step 3 — Tell Git who you are
This labels your commits with your name, so the team knows who did what:
```bash
git config --global user.name "Your Name"
git config --global user.email "your_email@example.com"
```
Use the same email as your GitHub account.

### Step 4 — Clone the repo
Navigate to the folder on your computer where you want the project to live, then run:
```bash
git clone <REPO_URL>
```
Replace `<REPO_URL>` with the link from the green "Code" button on our GitHub repo page. This creates a folder with the full project inside it.

```bash
cd chip-your-luck-firmware
```
(or whatever the folder is named) to move into it.

> **Prefer clicking over typing?** [GitHub Desktop](https://desktop.github.com/) is a free app that does everything in this guide through buttons instead of terminal commands. It's a completely valid way to work — feel free to use it if the command line feels intimidating at first. The concepts below (branches, commits, pull requests) are identical either way.

---

## 3. Our Branch Strategy

We use two permanent branches, plus temporary ones for individual work:

```
main   ──────●────────────────●───────────────●──────►   (always working — this is what
                                                            gets demoed/graded)
              \                \               \
dev    ────────●────●────●─────●───●────●──────●─────►   (integration branch — features
                                                            land here first)
                \        \           \
feature/xyz      ●────●   ●───●───●    ●──●              (your personal work-in-progress)
```

- **`main`** — Always contains working, tested code. Nothing gets committed to `main` directly. It only receives updates when `dev` has been tested and is ready for a milestone (Design Review, prototype demo, etc.).
- **`dev`** — The shared "in-progress" branch. This is where everyone's finished features get merged together and tested against each other before they're trusted enough for `main`.
- **`feature/your-thing`** — A short-lived branch just for you, while you build one specific piece (e.g. `feature/audio-driver`, `feature/led-cycling`). You branch this off of `dev`, do your work, then merge it back into `dev` when it's done.

**Why bother with all this?** If everyone edited `main` directly, one person's half-finished, broken code could stop everyone else from working. Branches let 5 people work in parallel without stepping on each other.

---

## 4. The Golden Rule

> **Never commit directly to `main`. Never commit directly to `dev` either — always work in your own `feature/` branch and merge in through a Pull Request.**

This means: before you write a single line of code, create a new branch. This one habit prevents almost every serious Git disaster.

---

## 5. Your Daily Workflow, Step by Step

Here's the full cycle, from opening your laptop to your code being merged in.

### Step 1 — Make sure you're up to date
Before starting any new work, get the latest version of `dev`:
```bash
git checkout dev
git pull
```
`checkout` switches you to a branch; `pull` downloads the latest changes for it.

### Step 2 — Create your own branch
```bash
git checkout -b feature/short-description
```
Example: `git checkout -b feature/volume-knob-driver`. The `-b` flag means "create this branch, then switch to it." Use lowercase words separated by hyphens, and make the name describe what you're building.

### Step 3 — Do your work
Edit files normally in your code editor (VS Code, etc.). Git is just watching in the background — it doesn't save anything permanently until you tell it to.

### Step 4 — Check what you changed
```bash
git status
```
This lists every file you've edited, added, or deleted since your last commit. Get in the habit of running this often — it's harmless and tells you exactly where you stand.

### Step 5 — Stage your changes
"Staging" means telling Git which changes you want to include in your next snapshot:
```bash
git add .
```
The `.` means "everything I changed." If you only want to include specific files: `git add path/to/file.c`.

### Step 6 — Commit
```bash
git commit -m "Add volume knob ADC reading and debounce"
```
The `-m` flag lets you attach a message. **Write messages that describe what changed and why**, not just "fixed stuff" — future-you and your teammates will thank you when scanning history for a bug.

### Step 7 — Push your branch to GitHub
```bash
git push -u origin feature/volume-knob-driver
```
The first time you push a new branch, include `-u origin branch-name` so Git remembers where it goes. After that, you can just type `git push`.

### Step 8 — Open a Pull Request
Go to the repo on GitHub.com — it will usually show a yellow banner offering to "Compare & pull request" for the branch you just pushed. Click it, make sure the base branch is `dev` (not `main`), write a short description of what you did, and submit it.

### Step 9 — Get it reviewed
Ask at least one teammate to look over your Pull Request before merging. This isn't about distrust — a second pair of eyes catches bugs before they land in `dev` where everyone depends on them.

### Step 10 — Merge
Once approved, click "Merge pull request" on GitHub. Your feature branch's changes are now part of `dev`. You can delete the feature branch afterward (GitHub will offer a button for this) — it's already served its purpose.

### Step 11 — Repeat
Go back to Step 1 for your next piece of work. Always start fresh from an updated `dev`.

---

## 6. Git Command Cheat Sheet

| Command | What it does |
|---|---|
| `git clone <url>` | Download the repo for the first time |
| `git status` | Show what's changed and what branch you're on |
| `git checkout dev` | Switch to the `dev` branch |
| `git checkout -b feature/name` | Create and switch to a new branch |
| `git pull` | Download the latest changes for your current branch |
| `git add .` | Stage all changed files for the next commit |
| `git add <file>` | Stage one specific file |
| `git commit -m "message"` | Save a snapshot of staged changes |
| `git push` | Upload your commits to GitHub |
| `git push -u origin branch-name` | Upload a brand-new branch for the first time |
| `git log --oneline` | See a short history of past commits |
| `git diff` | See exactly what lines you've changed but haven't committed yet |

---

## 7. Common Problems & How to Fix Them

**"I get an error when I try to push."**
Usually means someone else pushed changes to the same branch since you last pulled. Run `git pull` first, resolve any conflicts (see below), then push again.

**"I have a merge conflict and don't know what to do."**
Git will mark the conflicting section in the file directly, like this:
```
<<<<<<< HEAD
your version of the line
=======
their version of the line
>>>>>>> dev
```
Open the file, decide which version (or combination) is correct, delete the `<<<<<<<`, `=======`, and `>>>>>>>` marker lines, save the file, then:
```bash
git add .
git commit -m "Resolve merge conflict"
```
If this looks scary the first time, that's completely normal — ask a teammate to sit with you through your first one.

**"I committed to the wrong branch."**
Don't panic, nothing is lost. Ask in `#firmware` before doing anything — this is fixable but the fix depends on the specifics.

**"I want to undo my last commit but keep my changes."**
```bash
git reset --soft HEAD~1
```
This undoes the commit but leaves your edits in place, so you can fix the message or add more changes.

**"I want to throw away all my uncommitted changes and start over."**
```bash
git checkout -- .
```
Warning: this permanently deletes any changes you haven't committed. Only use it if you're sure.

**"It says I'm in a 'detached HEAD' state."**
This just means you're looking at an old commit instead of a branch. Run `git checkout dev` (or whichever branch you meant to be on) to get back to normal.

---

## 8. Getting Help

- Post in `#firmware` on Discord with what you were trying to do and the exact error message (screenshot is fine).
- `git status` is your best diagnostic tool — run it and read what it says before asking for help; it usually tells you exactly what state you're in.
- No question is too basic. Everyone on this team was a Git beginner once.
